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
    std::vector<std::string> rasterFallbackNames;

public:
    DataManager();
    ~DataManager();
    
    VectorLayer* loadVector(const std::string& path);
    RasterLayer* loadRaster(const std::string& path);
};


