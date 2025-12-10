#include <core/DataManager.hpp>
#include <core/GeoPackageReader.hpp>
#include <iostream>

/**
 * @file DataManager.cpp
 * @brief Implementation of DataManager class
 * 
 * Contains methods for loading vector data from files.
 */

/**
 * @brief Default constructor
 */
DataManager::DataManager() {}

/**
 * @brief Destructor
 */
DataManager::~DataManager() {}

/**
 * @brief Loads a vector layer from a GeoPackage file
 * @param chemin Path to GeoPackage file
 * @return Pointer to loaded VectorLayer
 * 
 * Opens the GeoPackage, reads the first layer,
 * extracts geometries, and creates a VectorLayer object.
 * Returns nullptr if file cannot be read.
 */
VectorLayer *DataManager::loadVector(const std::string &chemin)
{
    // Open GeoPackage file
    GeoPackageReader reader(chemin);
    if (!reader.open())
        return nullptr;

    // Get available layers
    auto couches = reader.listLayers();
    if (couches.empty())
        return nullptr;

    // Read metadata of first layer
    auto metadata = reader.getLayerMetadata(couches[0]);
    auto layer = std::make_shared<VectorLayer>(couches[0], metadata.crs, metadata.referenceEpoch);

    // Extract and add geometries to the layer
    auto features = reader.extractFeatures(couches[0]);
    for (const auto &f : features)
    {
        auto geom = std::make_shared<Geometry4D>(f.geometry);
        layer->addGeometry(geom);
    }
    
    // Close file and store the layer
    reader.close();
    vectorLayers.push_back(layer);

    return layer.get();
}