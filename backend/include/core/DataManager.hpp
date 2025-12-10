#pragma once

#include <string>
#include <memory>
#include <vector>
#include <core/VectorLayer.hpp>

/**
 * @class DataManager
 * @brief Manages vector data layers in the GIS
 * 
 * Loads and stores vector layers from various file formats.
 * Used by the main application to handle data operations.
 */
class DataManager {
private:
    /// List of loaded vector layers
    std::vector<std::shared_ptr<VectorLayer>> vectorLayers;

public:
    /**
     * @brief Default constructor
     */
    DataManager();
    
    /**
     * @brief Destructor
     */
    ~DataManager();
    
    /**
     * @brief Loads vector layers from a file
     * @param path File path (.shp, .gpkg, etc.)
     * @return Vector of pointers to loaded layers
     * 
     * Called when user imports a file through the interface.
     * Returns empty vector if file cannot be read.
     */
    std::vector<VectorLayer*> loadVector(const std::string& path);
};