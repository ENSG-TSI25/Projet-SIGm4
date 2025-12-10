#include <Application.hpp>
#include <core/DataManager.hpp>
#include <db/Config.hpp>
#include <gdal/gdal.h>
#include <iostream>

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
    
    std::cout << "\n=== Test Vector GeoPackage ===" << std::endl;
    DataManager dm;
    VectorLayer *layer = dm.loadVector("/app/data/cantonsVendee_EPSG2154_15-09-2025_clean.gpkg");
    
    if (layer)
    {
        std::cout << "Layer: " << layer->getName() << std::endl;
        std::cout << "CRS: " << layer->getCrs() << std::endl;
        std::cout << "Epoch: " << layer->getEpoch() << std::endl;
        
        const auto &geoms = layer->getGeometries();
        if (!geoms.empty())
        {
            const auto &geom = geoms[0];
            std::cout << "\n--- First geometry ---" << std::endl;
            
            OGRGeometry *g = geom->getGeometry();
            OGRwkbGeometryType type = wkbFlatten(g->getGeometryType());
            
            OGRPoint pt;
            double t = geom->getT();
            
            if (type == wkbPolygon)
            {
                g->toPolygon()->getExteriorRing()->getPoint(0, &pt);
            }
            else if (type == wkbMultiPolygon)
            {
                g->toMultiPolygon()->getGeometryRef(0)->getExteriorRing()->getPoint(0, &pt);
            }
            
            std::cout << "First point (X, Y, Z, T): ("
                      << pt.getX() << ", " << pt.getY() << ", "
                      << pt.getZ() << ", " << t << ")" << std::endl;
        }
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

    std::cout << "\n✓ Application running successfully!" << std::endl;
}