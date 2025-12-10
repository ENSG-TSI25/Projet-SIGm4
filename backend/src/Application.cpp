#include <Application.hpp>
#include <core/DataManager.hpp>
#include <db/Config.hpp>
#include <gdal/gdal.h>
#include <iostream>

/**
 * @file Application.cpp
 * @brief Implementation of Application class
 * 
 * Main backend application logic.
 */

/**
 * @brief Constructor - loads DB config from environment
 */
Application::Application()
{
    DBConfig cfg = DBConfig::loadFromEnv();
    dbClient = std::make_unique<PostgresClient>(cfg);
}

/**
 * @brief Initializes application components
 * 
 * Registers GDAL drivers, connects to database,
 * and initializes PostGIS extensions.
 */
void Application::initialize()
{
    std::cout << "=== Initializing Application ===" << std::endl;
    GDALAllRegister();
    std::cout << "GDAL version: " << GDALVersionInfo("VERSION_NUM") << std::endl;
    
    dbClient->connect();
    dbClient->initExtensions();
}

/**
 * @brief Main application execution
 * 
 * Tests database connection, loads sample data,
 * and displays geometry information.
 */
void Application::run()
{
    std::cout << "\n=== Running Application ===" << std::endl;
    pqxx::result r = dbClient->execute("SELECT PostGIS_Version();");
    std::cout << "PostGIS version: " << r[0][0].c_str() << std::endl;
    
    std::cout << "\n=== Test GeoPackage ===" << std::endl;
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
    
    std::cout << "\n✓ Application running successfully!" << std::endl;
}