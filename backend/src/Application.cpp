#include <Application.hpp>
#include <core/DataManager.hpp>
#include <db/Config.hpp>
#include <gdal/gdal.h>
#include <iostream>

Application::Application() {
    DBConfig cfg = DBConfig::loadFromEnv();
    dbClient = std::make_unique<PostgresClient>(cfg);
}

void Application::initialize() {
    std::cout << "=== Initializing Application ===" << std::endl;
    GDALAllRegister();
    std::cout << "GDAL version: " << GDALVersionInfo("VERSION_NUM") << std::endl;
    
    dbClient->connect();
    dbClient->initExtensions();
}

void Application::run() {
    std::cout << "\n=== Running Application ===" << std::endl;
    
    pqxx::result r = dbClient->execute("SELECT PostGIS_Version();");
    std::cout << "PostGIS version: " << r[0][0].c_str() << std::endl;
    
    std::cout << "\n=== Test GeoPackage ===" << std::endl;
    
    DataManager dm;
    VectorLayer* layer = dm.chargerVecteur("/app/data/cantonsVendee_EPSG2154_15-09-2025_clean.gpkg");
    
    
    if (layer) {
        std::cout << "Couche: " << layer->getNom() << std::endl;
        std::cout << "CRS: " << layer->getCrs() << std::endl;
        std::cout << "Nombre géométries: " << layer->getNombreGeometries() << std::endl;
        
        auto geoms = layer->obtenirGeometries();
        if (!geoms.empty()) {
            auto geom = geoms[0];
            std::cout << "Première géométrie - Timestamp: " << geom->getT() << std::endl;
        }
    }
    
    std::cout << "\n✓ Application running successfully!" << std::endl;
}