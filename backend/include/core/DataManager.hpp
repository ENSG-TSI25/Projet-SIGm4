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
    
    std::vector<VectorLayer*> loadVector(const std::string& path);
};