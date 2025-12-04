#pragma once
#include <core/Layer.hpp>
#include <core/Geometry4D.hpp>
#include <vector>
#include <memory>

class VectorLayer : public Layer {
private:
    std::vector<std::shared_ptr<Geometry4D>> geometries;

public:
    VectorLayer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", double epoque_ = 0.0)
        : Layer(nom_, crs_, epoque_) {}
    
    void addGeometry(std::shared_ptr<Geometry4D> geom) {
        geometries.push_back(geom);
    }
    
    
    const std::vector<std::shared_ptr<Geometry4D>>& getGeometries() const {
        return geometries;
    }
    

};