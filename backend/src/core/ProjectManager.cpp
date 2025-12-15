#include "core/ProjectManager.hpp"
#include "core/VectorLayer.hpp"
#include "core/RasterLayer.hpp"
#include <iostream>
#include <stdexcept>

ProjectManager::ProjectManager(const Project& project_)
    : project(project_), engine(std::make_unique<TransformationEngine>()) {}

ProjectManager::~ProjectManager() = default;


std::vector<std::shared_ptr<Layer>> ProjectManager::applyProjectParameters() {
    std::vector<std::shared_ptr<Layer>> transformedLayers;
    auto layers = project.getLayers();
    const std::string target_crs = project.getCrs();
    const std::string target_epsg = target_crs.substr(target_crs.find(":") + 1);
    
    for (auto& layerPtr : layers) {
        if (layerPtr->getCrs() == target_crs) {
            transformedLayers.push_back(layerPtr);
            continue;
        }
        
        if (auto vecLayer = std::dynamic_pointer_cast<VectorLayer>(layerPtr)) {
            try {
                // Transform modifies layer in-place, ignore return value
                engine->transformLayerAtEpoch(*vecLayer, target_epsg);
                transformedLayers.push_back(layerPtr);
                std::cout << "Transformed layer: " << layerPtr->getName() 
                          << " from " << layerPtr->getCrs() 
                          << " to " << target_crs << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error transforming layer " << layerPtr->getName() 
                          << ": " << e.what() << std::endl;
                transformedLayers.push_back(layerPtr);
            }
        }
        else if (std::dynamic_pointer_cast<RasterLayer>(layerPtr)) {
            std::cerr << "Warning: RasterLayer " << layerPtr->getName() 
                      << " transformation not yet implemented, skipping" << std::endl;
            transformedLayers.push_back(layerPtr);
        }
        else {
            std::cerr << "Warning: Unknown layer type for " << layerPtr->getName() 
                      << ", skipping" << std::endl;
            transformedLayers.push_back(layerPtr);
        }
    }
    
    return transformedLayers;
}