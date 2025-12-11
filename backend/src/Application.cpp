#include <Application.hpp>
#include <core/DataManager.hpp>
#include <db/Config.hpp>
#include <gdal/gdal.h>
#include <iostream>
#include <fstream>         
#include <cmath>           
#include <core/Project.hpp> 
#include <core/Layer.hpp> 
Application::Application()
{
    DBConfig cfg = DBConfig::loadFromEnv();
    dbClient = std::make_unique<PostgresClient>(cfg);
}

void Application::initialize()
{
    std::cout << "=== Initializing Application ===" << std::endl;
    GDALAllRegister();
    std::cout << "GDAL version: " << GDALVersionInfo("VERSION_NUM") << std::endl;
    
    dbClient->connect();
    dbClient->initExtensions();
}

void Application::run()
{
    std::cout << "\n=== Running Application ===" << std::endl;
    pqxx::result r = dbClient->execute("SELECT PostGIS_Version();");
    std::cout << "PostGIS version: " << r[0][0].c_str() << std::endl;
    
    std::cout << "=== TEST LOADVECTOR WITH ALL LAYERS ===" << std::endl;
    
    DataManager dm;
    
    // Test with your data file
    std::string filePath = "/app/backend/data/cantonsVendee_EPSG2154_15-09-2025_clean.gpkg";
    auto layers = dm.loadVector(filePath);
    
    if (layers.empty()) {
        std::cout << "No layers loaded!" << std::endl;
        return;
    }
    
    std::cout << "Successfully loaded " << layers.size() << " layer(s)" << std::endl;
    
    // Test each layer
    for (size_t i = 0; i < layers.size(); ++i) {
        auto* layer = layers[i];
        
        std::cout << "LAYER " << (i + 1) << " INFORMATION" << std::endl;
        
        // Layer metadata
        std::cout << "Name: " << layer->getName() << std::endl;
        std::cout << "CRS: " << layer->getCrs() << std::endl;
        std::cout << "Reference Epoch: " << layer->getEpoch() << std::endl;
        
        // Get geometries and EWKT
        const auto& geoms = layer->getGeometries();
        const auto& ewkts = layer->getEWKT();
        
        std::cout << "Number of geometries: " << geoms.size() << std::endl;
        std::cout << "Number of EWKT strings: " << ewkts.size() << std::endl;
        
        if (geoms.empty()) {
            std::cout << "No geometries in this layer" << std::endl;
            continue;
        }
        
        // Display first 3 geometries
        std::cout << "\n--- First " << std::min(size_t(3), geoms.size()) << " geometry details ---" << std::endl;
        
        for (size_t j = 0; j < std::min(size_t(3), geoms.size()); ++j) {
            std::cout << "\nGeometry [" << j << "]:" << std::endl;
            
            auto geom = geoms[j];
            
            // SRID and timestamp
            std::cout << "  SRID: " << geom->getSRID() << std::endl;
            std::cout << "  Timestamp (T): " << geom->getT() << std::endl;
            
            // Geometry type
            OGRGeometry* g = geom->getGeometry();
            if (g) {
                std::cout << "  Type: " << g->getGeometryName() << std::endl;
                
                OGRwkbGeometryType type = wkbFlatten(g->getGeometryType());
                
                // Get first coordinate
                if (type == wkbPoint) {
                    OGRPoint* pt = g->toPoint();
                    std::cout << "  Coordinates: X=" << pt->getX() 
                              << ", Y=" << pt->getY() 
                              << ", Z=" << pt->getZ() 
                              << ", M=" << pt->getM() << std::endl;
                }
                else if (type == wkbLineString) {
                    OGRLineString* line = g->toLineString();
                    std::cout << "  First point: X=" << line->getX(0) 
                              << ", Y=" << line->getY(0) 
                              << ", Z=" << line->getZ(0) 
                              << ", M=" << line->getM(0) << std::endl;
                    std::cout << "  Total points: " << line->getNumPoints() << std::endl;
                }
                else if (type == wkbPolygon) {
                    OGRPolygon* poly = g->toPolygon();
                    OGRLinearRing* ring = poly->getExteriorRing();
                    std::cout << "  First point: X=" << ring->getX(0) 
                              << ", Y=" << ring->getY(0) 
                              << ", Z=" << ring->getZ(0) 
                              << ", M=" << ring->getM(0) << std::endl;
                    std::cout << "  Total points in exterior ring: " << ring->getNumPoints() << std::endl;
                }
                else if (type == wkbMultiPolygon) {
                    OGRMultiPolygon* mpoly = g->toMultiPolygon();
                    std::cout << "  Number of polygons: " << mpoly->getNumGeometries() << std::endl;
                    if (mpoly->getNumGeometries() > 0) {
                        OGRPolygon* firstPoly = static_cast<OGRPolygon*>(mpoly->getGeometryRef(0));
                        OGRLinearRing* ring = firstPoly->getExteriorRing();
                        std::cout << "  First polygon first point: X=" << ring->getX(0) 
                                  << ", Y=" << ring->getY(0) 
                                  << ", Z=" << ring->getZ(0) 
                                  << ", M=" << ring->getM(0) << std::endl;
                    }
                }
            }
            
            std::string ewkt = ewkts[j];
            if (ewkt.length() > 150) {
                ewkt = ewkt.substr(0, 150) + "...";
            }
            std::cout << "  EWKT: " << ewkt << std::endl;
        }


    std::cout << "=======================================" << std::endl;
    std::cout << "=== TEST PROJECT SAVE/LOAD (.sigm4) ===" << std::endl;
    std::cout << "=======================================" << std::endl;

    // Créer un projet de test
    std::cout << "\nCréation d'un projet test..." << std::endl;
    Project testProject("Projet Test Vendée", 2025.0, "EPSG:2154");
    
    // Ajouter des couches avec les données chargées
    for (size_t i = 0; i < layers.size(); ++i) {
        auto* vectorLayer = layers[i];
        Layer simpleLayer(
            vectorLayer->getName(),
            vectorLayer->getCrs(),
            vectorLayer->getEpoch()
        );
        testProject.addLayer(simpleLayer);
        std::cout << "Couche ajoutée: " << vectorLayer->getName() << std::endl;
    }
    
    std::cout << "Projet créé avec " << testProject.getLayers().size() << " couches" << std::endl;
    
    // Sauvegarder le projet
    std::cout << "\nSauvegarde du projet..." << std::endl;
    std::string projectPath = "/app/backend/data/test_project.sigm4";
    bool saveSuccess = testProject.save(projectPath);
    
    if (saveSuccess) {
        std::cout << "Projet sauvegardé: " << projectPath << std::endl;
    } else {
        std::cout << "Échec de la sauvegarde" << std::endl;
        return;
    }
        
 
    std::cout << "\n=== Test Raster GeoPackage ===" << std::endl;
    RasterLayer* raster = dm.loadRaster("/app/data/raster_data.gpkg");
      
      
      
      
      
      
    if (raster) {
        // Attributs Layer
        std::cout << "Name: " << raster->getName() << std::endl;
        std::cout << "CRS: " << raster->getCrs() << std::endl;
        std::cout << "Epoch: " << raster->getEpoch() << std::endl;
        
        // Attributs RasterLayer
        std::cout << "Dimensions: " << raster->getWidth() << "x" << raster->getHeight() << " pixels" << std::endl;
        std::cout << "FilePath: " << raster->getFilePath() << std::endl;
        
        // GeoTransform
        const double* gt = raster->getGeoTransform();
        std::cout << "GeoTransform: [" << gt[0] << ", " << gt[1] << ", " << gt[2] << ", "
                  << gt[3] << ", " << gt[4] << ", " << gt[5] << "]" << std::endl;
        std::cout << "Resolution: " << gt[1] << "m/pixel (X), " << std::abs(gt[5]) << "m/pixel (Y)" << std::endl;
        
        // Emprise
        auto emprise = raster->getEmprise();
        if (emprise) {
            std::cout << "Extent EWKT: " << emprise->toEWKT() << std::endl;
        }
    }
      
      
      
    
    // Charger le projet
    std::cout << "\nChargement du projet sauvegardé..." << std::endl;
    try {
        Project loadedProject = Project::load(projectPath);
        
        std::cout << "Projet chargé avec succès!" << std::endl;
        std::cout << "\n Vérification des données:" << std::endl;
        std::cout << "   - Nom: " << loadedProject.getName() << std::endl;
        std::cout << "   - CRS: " << loadedProject.getCrs() << std::endl;
        std::cout << "   - Époque: " << loadedProject.getEpoch0() << std::endl;
        std::cout << "   - Nombre de couches: " << loadedProject.getLayers().size() << std::endl;
        
   
        
        // Afficher le contenu du fichier JSON
        std::cout << "\nContenu du fichier .sigm4:" << std::endl;
        std::cout << "   -----------------------------------" << std::endl;
        std::ifstream file(projectPath);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                std::cout << "   " << line << std::endl;
            }
            file.close();
        }
        
    
        
    } catch (const std::exception& e) {
        std::cout << "   Erreur lors du chargement: " << e.what() << std::endl;
    }




    }

    std::cout << "\n✓ Application running successfully!" << std::endl;
}