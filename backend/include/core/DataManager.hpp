#pragma once
#include <string>
#include <memory>
#include <vector>
#include <core/VectorLayer.hpp>
#include <core/RasterLayer.hpp>

class DataManager {
private:
    std::vector<std::shared_ptr<VectorLayer>> vectorLayers;
    std::vector<std::shared_ptr<RasterLayer>> rasterLayers;
public:
    DataManager();
    ~DataManager();
    
    RasterLayer* loadRaster(const std::string& path);
    std::vector<VectorLayer*> loadVector(const std::string& path);
};