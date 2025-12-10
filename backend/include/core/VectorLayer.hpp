#pragma once

#include <core/Layer.hpp>
#include <core/Geometry4D.hpp>
#include <vector>
#include <memory>

/**
 * @class VectorLayer
 * @brief Vector layer containing geometries
 * 
 * Extends Layer class with vector geometry support.
 */
class VectorLayer : public Layer {
private:
    std::vector<std::shared_ptr<Geometry4D>> geometries;

public:
    /**
     * @brief Constructor
     * @param nom_ Layer name
     * @param crs_ Coordinate system
     * @param epoque_ Reference epoch
     */
    VectorLayer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", double epoque_ = 0.0)
        : Layer(nom_, crs_, epoque_) {}
    
    /**
     * @brief Adds a geometry to the layer
     * @param geom Geometry to add
     */
    void addGeometry(std::shared_ptr<Geometry4D> geom) {
        geometries.push_back(geom);
    }
    
    /**
     * @brief Gets all geometries in the layer
     * @return Vector of geometry pointers
     */
    const std::vector<std::shared_ptr<Geometry4D>>& getGeometries() const {
        return geometries;
    }
};