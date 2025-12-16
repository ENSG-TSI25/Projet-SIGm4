#include "core/ProjectManager.hpp"
#include "core/VectorLayer.hpp"
#include "core/RasterLayer.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <gdal/gdal.h>
#include <gdal/ogrsf_frmts.h>

ProjectManager::ProjectManager(const Project& project_)
    : project(project_), engine(std::make_unique<TransformationEngine>()) {}

ProjectManager::~ProjectManager() = default;

std::vector<std::string> ProjectManager::applyProjectParameters() {
    std::vector<std::string> outputPaths;
    auto layers = project.getLayers();
    
    const std::string target_crs = project.getCrs();
    const std::string target_epsg = target_crs.substr(target_crs.find(":") + 1);
    
    // Get timestamp once for all files
    std::string timestamp = getCurrentTimestamp();
    
    // Get path mapping from environment variables
    const char* outputDirHost = std::getenv("OUTPUT_DIR_HOST");
    const char* outputDirContainer = std::getenv("OUTPUT_DIR_CONTAINER");
    std::string hostPath = outputDirHost ? outputDirHost : "";
    std::string containerPath = outputDirContainer ? outputDirContainer : "/tmp";
    
    for (auto& layerPtr : layers) {
        if (layerPtr->getCrs() == target_crs) {
            std::cout << "Layer " << layerPtr->getName() 
                     << " already in target CRS, skipping" << std::endl;
            continue;
        }
        
        if (auto vecLayer = std::dynamic_pointer_cast<VectorLayer>(layerPtr)) {
            try {
                // Get original file path
                std::string originalPath = vecLayer->getDataSource();
                
                // Generate output in /tmp (mapped to host)
                std::string outputDir = containerPath;
                
                // Create directory if it doesn't exist
                std::filesystem::create_directories(outputDir);
                
                std::string containerOutputPath = outputDir + "/" + vecLayer->getName() + 
                                       "_transformed_" + target_epsg + "_" + 
                                       timestamp + ".gpkg";
                
                // Check if source file exists (file-based layer)
                if (!originalPath.empty() && std::filesystem::exists(originalPath)) {
                    // Copy original file
                    std::filesystem::copy_file(originalPath, containerOutputPath, 
                        std::filesystem::copy_options::overwrite_existing);
                    
                    // Transform layer in-place
                    engine->transformLayerAtEpoch(*vecLayer, target_epsg);
                    
                    // Update the copied GeoPackage with transformed coordinates
                    updateGeoPackageWithTransformedData(containerOutputPath, vecLayer, target_crs);
                } else {
                    // In-memory layer - create new GeoPackage
                    engine->transformLayerAtEpoch(*vecLayer, target_epsg);
                    createGeoPackageFromLayer(containerOutputPath, vecLayer, target_crs);
                }
                
                // Map container path to host path
                std::string hostOutputPath = containerOutputPath;
                if (!hostPath.empty() && !containerPath.empty()) {
                    hostOutputPath = hostPath + "/" + vecLayer->getName() + 
                                   "_transformed_" + target_epsg + "_" + 
                                   timestamp + ".gpkg";
                }
                
                // Add HOST path to results
                outputPaths.push_back(hostOutputPath);
                
                std::cout << "Transformed layer: " << layerPtr->getName() 
                         << " from " << layerPtr->getCrs() 
                         << " to " << target_crs 
                         << "\n  Output (host): " << hostOutputPath << std::endl;
                         
            } catch (const std::exception& e) {
                std::cerr << "Error transforming layer " << layerPtr->getName() 
                         << ": " << e.what() << std::endl;
            }
        }
        else if (std::dynamic_pointer_cast<RasterLayer>(layerPtr)) {
            std::cerr << "Warning: RasterLayer " << layerPtr->getName() 
                     << " transformation not yet implemented, skipping" << std::endl;
        }
        else {
            std::cerr << "Warning: Unknown layer type for " << layerPtr->getName() 
                     << ", skipping" << std::endl;
        }
    }
    
    return outputPaths;
}


void ProjectManager::createGeoPackageFromLayer(
    const std::string& gpkgPath,
    const std::shared_ptr<VectorLayer>& layer,
    const std::string& targetCrs) {
    
    // Create new GeoPackage
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
    if (!driver) {
        throw std::runtime_error("GPKG driver not available");
    }
    
    GDALDataset* dataset = driver->Create(gpkgPath.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    if (!dataset) {
        throw std::runtime_error("Cannot create GeoPackage: " + gpkgPath);
    }
    
    // Create spatial reference
    OGRSpatialReference srs;
    std::string epsg = targetCrs.substr(targetCrs.find(":") + 1);
    srs.importFromEPSG(std::stoi(epsg));
    
    // Create layer
    OGRLayer* ogrLayer = dataset->CreateLayer(
        layer->getName().c_str(),
        &srs,
        wkbPoint,
        nullptr);
    
    if (!ogrLayer) {
        GDALClose(dataset);
        throw std::runtime_error("Cannot create layer in GeoPackage");
    }
    
    // Add features
    const auto& geometries = layer->getGeometries();
    for (size_t i = 0; i < geometries.size(); ++i) {
        OGRFeature* feature = OGRFeature::CreateFeature(ogrLayer->GetLayerDefn());
        
        // Clone geometry
        OGRGeometry* geom = geometries[i]->getGeometry()->clone();
        feature->SetGeometry(geom);
        
        if (ogrLayer->CreateFeature(feature) != OGRERR_NONE) {
            std::cerr << "Failed to create feature " << i << std::endl;
        }
        
        OGRFeature::DestroyFeature(feature);
    }
    
    GDALClose(dataset);
}

std::string ProjectManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S");
    return ss.str();
}

std::string ProjectManager::generateOutputFilename(
    const std::string& layerName,
    const std::string& targetEpsg,
    const std::string& timestamp) {
    
    // Keep .gpkg extension
    return "output/transformed/" + layerName + "_transformed_" + 
           targetEpsg + "_" + timestamp + ".gpkg";
}

void ProjectManager::updateGeoPackageWithTransformedData(
    const std::string& gpkgPath,
    const std::shared_ptr<VectorLayer>& layer,
    const std::string& targetCrs) {
    
    GDALDataset* dataset = (GDALDataset*)GDALOpenEx(
        gpkgPath.c_str(), 
        GDAL_OF_UPDATE | GDAL_OF_VECTOR, 
        nullptr, nullptr, nullptr);
    
    if (!dataset) {
        throw std::runtime_error("Cannot open GeoPackage for update: " + gpkgPath);
    }
    
    // Get the layer
    OGRLayer* ogrLayer = dataset->GetLayerByName(layer->getName().c_str());
    if (!ogrLayer) {
        GDALClose(dataset);
        throw std::runtime_error("Layer not found in GeoPackage: " + layer->getName());
    }
    
    // Update geometries
    const auto& geometries = layer->getGeometries();
    ogrLayer->ResetReading();
    
    for (size_t i = 0; i < geometries.size(); ++i) {
        OGRFeature* feature = ogrLayer->GetNextFeature();
        if (!feature) break;
        
        // Clone the transformed geometry
        OGRGeometry* newGeom = geometries[i]->getGeometry()->clone();
        feature->SetGeometry(newGeom);
        
        // Update feature
        if (ogrLayer->SetFeature(feature) != OGRERR_NONE) {
            std::cerr << "Failed to update feature " << i << std::endl;
        }
        
        OGRFeature::DestroyFeature(feature);
    }
    
    GDALClose(dataset);
}