#include <Application.hpp>
#include <core/DataManager.hpp>
#include <db/Config.hpp>
#include <gdal/gdal.h>
#include <iostream>
#include <fstream>
#include <iomanip> 
#include <cmath>
#include <core/Project.hpp>
#include <core/Layer.hpp>
#include <core/ProjectManager.hpp>
#include <core/VectorLayer.hpp>
#include <core/RasterLayer.hpp>


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

    std::cout << "\n=== TEST LOADVECTOR WITH ALL LAYERS ===" << std::endl;

    DataManager dm;
    std::string filePath = "/app/backend/data/pointsEau.gpkg";
    auto layers = dm.loadVector(filePath);

    if (layers.empty())
    {
        std::cout << "No layers loaded!" << std::endl;
        return;
    }

    std::cout << "Successfully loaded " << layers.size() << " layer(s)" << std::endl;

    for (size_t i = 0; i < layers.size(); ++i)
    {
        auto *layer = layers[i];

        std::cout << "\n=== LAYER " << (i + 1) << ": " << layer->getName() << " ===" << std::endl;
        std::cout << "CRS: " << layer->getCrs() << std::endl;
        std::cout << "Reference Epoch: " << layer->getEpoch() << std::endl;

        // TEST TEMPORAL DATA
        std::cout << "Has Temporal Data: " << (layer->hasTemporalData() ? "YES" : "NO") << std::endl;
        if (layer->hasTemporalData())
        {
            std::cout << "Timestamp Field: " << layer->getTimestampField() << std::endl;

            const auto &geoms = layer->getGeometries();
            std::cout << "\nFirst 3 features with attributes:" << std::endl;
            for (size_t j = 0; j < std::min(size_t(3), geoms.size()); ++j)
            {
                std::cout << "  Feature [" << j << "]:" << std::endl;
                std::cout << "    Timestamp (T): " << geoms[j]->getT() << std::endl;
                auto attrs = layer->getFeatureAttributes(j);
                for (const auto &[key, value] : attrs)
                {
                    std::cout << "    " << key << ": " << value << std::endl;
                }
            }
        }

        std::cout << "Total geometries: " << layer->getGeometries().size() << std::endl;
    }

    std::cout << "\n=== TEST PROJECT SAVE/LOAD ===" << std::endl;
    Project testProject("Test Project", 2025.0, "EPSG:4326");

    for (size_t i = 0; i < layers.size(); ++i)
    {
        auto *vectorLayer = layers[i];
        // Create VectorLayer instead of generic Layer
        auto vLayer = std::make_shared<VectorLayer>(
            vectorLayer->getName(),
            vectorLayer->getCrs(),
            vectorLayer->getEpoch(),
            vectorLayer->getDataSource());
        
        // Copy geometries
        for (const auto& geom : vectorLayer->getGeometries()) {
            vLayer->addGeometry(geom);
        }
        
        testProject.addLayer(vLayer);
    }

    std::string projectPath = "/app/backend/data/test_project.sigm4";
    bool saveSuccess = testProject.save(projectPath);

    if (saveSuccess)
    {
        std::cout << "✓ Project saved: " << projectPath << std::endl;
        
        // Use testProject directly instead of reloading
        std::cout << "\n=== TEST PROJECT TRANSFORMATION ===" << std::endl;
        ProjectManager projectManager(testProject);
        
        try {
            std::vector<std::string> transformedFiles = projectManager.applyProjectParameters();
            
            std::cout << "\n✓ Transformation completed!" << std::endl;
            std::cout << "Generated " << transformedFiles.size() << " transformed file(s):" << std::endl;
            
            for (const auto& filePath : transformedFiles) {
                std::cout << "  - " << filePath << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "✗ Transformation failed: " << e.what() << std::endl;
        }
    }
}

