#pragma once
#include <string>
#include <memory>
#include <vector>
#include <core/VectorLayer.hpp>

class DataManager {
private:
    std::vector<std::shared_ptr<VectorLayer>> vectorLayers;

public:
    DataManager();
    ~DataManager();
    
    VectorLayer* loadVector(const std::string& path);
};