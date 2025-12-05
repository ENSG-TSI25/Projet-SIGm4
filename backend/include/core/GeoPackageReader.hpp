#pragma once
#include <core/Geometry4D.hpp>
#include <string>
#include <vector>
#include <map>
#include <gdal/ogr_core.h>

class GDALDataset;
class OGRFeature;

class GeoPackageReader {
public:
    struct LayerMetadata {
        std::string name;
        std::string crs;
        int srid;
        size_t featureCount;
        double referenceEpoch;
        OGRwkbGeometryType geometryType;
    };

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
    GeoPackageReader(const std::string& path);
    ~GeoPackageReader();

    bool open();
    void close();
    std::vector<std::string> listLayers() const;
    LayerMetadata getLayerMetadata(const std::string& layerName) const;
    std::vector<Feature> extractFeatures(const std::string& layerName) const;

private:
    std::string detectTimestampField(const std::string& layerName) const;
    Feature convertOGRFeature(OGRFeature* ogrFeature, const std::string& timestampField) const;
    Geometry4D extractGeometry4D(OGRFeature* ogrFeature, double timestamp) const;
    double parseTimestamp(const std::string& timestampStr) const;
};