#pragma once

#include <core/Geometry4D.hpp>
#include <string>
#include <vector>
#include <map>
#include <gdal/ogr_core.h>

class GDALDataset;
class OGRFeature;

/**
 * @class GeoPackageReader
 * @brief Reads GeoPackage files
 * 
 * Handles opening, reading metadata, and extracting features
 * from GeoPackage format files.
 */
class GeoPackageReader {
public:
    /**
     * @struct LayerMetadata
     * @brief Metadata for a GeoPackage layer
     */
    struct LayerMetadata {
        std::string name;
        std::string crs;
        int srid;
        size_t featureCount;
        double referenceEpoch;
        OGRwkbGeometryType geometryType;
    };

    /**
     * @struct Feature
     * @brief Represents a feature with geometry and attributes
     */
    struct Feature {
        int fid;
        Geometry4D geometry;
        std::map<std::string, std::string> attributes;
        double timestamp;
    };

private:
    std::string filePath;
    GDALDataset* dataset;
    bool isOpen;

public:
    /**
     * @brief Constructor
     * @param path Path to GeoPackage file
     */
    GeoPackageReader(const std::string& path);
    
    /**
     * @brief Destructor
     */
    ~GeoPackageReader();

    /**
     * @brief Opens the GeoPackage file
     * @return true if successful
     */
    bool open();
    
    /**
     * @brief Closes the GeoPackage file
     */
    void close();
    
    /**
     * @brief Lists all layers in the file
     * @return Vector of layer names
     */
    std::vector<std::string> listLayers() const;
    
    /**
     * @brief Gets metadata for a specific layer
     * @param layerName Name of the layer
     * @return LayerMetadata structure
     */
    LayerMetadata getLayerMetadata(const std::string& layerName) const;
    
    /**
     * @brief Extracts features from a layer
     * @param layerName Name of the layer
     * @return Vector of Feature objects
     */
    std::vector<Feature> extractFeatures(const std::string& layerName) const;

private:
    std::string detectTimestampField(const std::string& layerName) const;
    Feature convertOGRFeature(OGRFeature* ogrFeature, const std::string& timestampField) const;
    Geometry4D extractGeometry4D(OGRFeature* ogrFeature, double timestamp) const;
    double parseTimestamp(const std::string& timestampStr) const;
};