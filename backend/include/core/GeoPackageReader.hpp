#pragma once
#include <core/Geometry4D.hpp>
#include <string>
#include <vector>
#include <map>
#include <gdal/ogr_core.h>

#include <sstream>
#include <iomanip>
#include <gdal/gdal.h>

class GDALDataset;
class OGRFeature;

class GeoPackageReader
{
public:
    struct LayerMetadata
    {
        std::string name;
        std::string crs;
        int srid;
        size_t featureCount;
        double referenceEpoch;
        OGRwkbGeometryType geometryType;
    };

    struct Feature
    {
        int fid;
        Geometry4D geometry;
        std::map<std::string, std::string> attributes;
        double timestamp;
    };

    struct RasterMetadata
    {
        std::string name;
        std::string crs;
        int srid;
        int width, height;
        double referenceEpoch;
        double geoTransform[6];
        std::string layerPath;
    };

private:
    std::string filePath;
    GDALDataset *dataset;
    bool isOpen;

public:
    GeoPackageReader(const std::string &path);
    ~GeoPackageReader();
    bool open();
    void close();
    std::vector<std::string> listLayers() const;
    LayerMetadata getLayerMetadata(const std::string &layerName) const;
    std::vector<Feature> extractFeatures(const std::string &layerName) const;
    bool isRasterLayer(const std::string &layerName) const;
    RasterMetadata extractRasterMetadata(const std::string &layerName) const;
    Geometry4D extractRasterExtent(const std::string &layerName, double timestamp) const;
    std::string detectTimestampField(const std::string &layerName) const;
    bool addTemporalField(const std::string& layerName, const std::string& fieldName, double defaultValue);

private:
    Feature convertOGRFeature(OGRFeature *ogrFeature, const std::string &timestampField) const;
    Geometry4D extractGeometry4D(OGRFeature *ogrFeature, double timestamp) const;
    double parseTimestamp(const std::string &timestampStr) const;

};