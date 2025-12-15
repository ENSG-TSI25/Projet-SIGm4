#include <core/GeoPackageReader.hpp>
#include <gdal/ogrsf_frmts.h>
#include <gdal/gdal.h>
#include <gdal/ogr_geometry.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <algorithm>

GeoPackageReader::GeoPackageReader(const std::string& path)
    : filePath(path), dataset(nullptr), isOpen(false) {
    GDALAllRegister(); // Initialisation GDAL
}

GeoPackageReader::~GeoPackageReader() { close(); }

bool GeoPackageReader::open() {
    if (isOpen) return true;
    
    // Try opening as vector first
    dataset = (GDALDataset*)GDALOpenEx(
        filePath.c_str(), 
        GDAL_OF_VECTOR | GDAL_OF_READONLY, 
        nullptr, nullptr, nullptr
    );
    
    // If fails, try as update mode to access both vector and raster
    if (!dataset) {
        dataset = (GDALDataset*)GDALOpenEx(
            filePath.c_str(), 
            GDAL_OF_READONLY,  // Remove GDAL_OF_VECTOR restriction
            nullptr, nullptr, nullptr
        );
    }
    
    return (isOpen = dataset != nullptr);
}

void GeoPackageReader::close() {
    // Fermeture du dataset
    if (dataset) {
        GDALClose(dataset);
        dataset = nullptr;
        isOpen = false;
    }
}

std::vector<std::string> GeoPackageReader::listLayers() const {
    // Récupère les noms des couches
    std::vector<std::string> layers;
    if (!isOpen) return layers;

    for (int i = 0; i < dataset->GetLayerCount(); ++i) {
        layers.push_back(dataset->GetLayer(i)->GetName());
    }
    return layers;
}

GeoPackageReader::LayerMetadata GeoPackageReader::getLayerMetadata(const std::string& layerName) const {
    // Lit les métadonnées d'une couche
    LayerMetadata m{"", "", 0, 0, 0.0, wkbUnknown};
    if (!isOpen) return m;
    
    OGRLayer* layer = dataset->GetLayerByName(layerName.c_str());
    if (!layer) return m;

    m.name = layerName;
    m.featureCount = layer->GetFeatureCount();
    m.geometryType = layer->GetGeomType();

    if (auto* srs = layer->GetSpatialRef()) {
        if (auto* code = srs->GetAuthorityCode(nullptr)) {
            m.srid = std::atoi(code);
            m.crs = std::string(srs->GetAuthorityName(nullptr)) + ":" + code;
        }
    }
    return m;
}

std::vector<GeoPackageReader::Feature> GeoPackageReader::extractFeatures(const std::string& layerName) const {
    // Extraction des features
    std::vector<Feature> features;
    if (!isOpen) return features;

    OGRLayer* layer = dataset->GetLayerByName(layerName.c_str());
    if (!layer) return features;

    std::string tsField = detectTimestampField(layerName);
    layer->ResetReading();
    
    OGRFeature* ogrf;
    while ((ogrf = layer->GetNextFeature())) {
        features.push_back(convertOGRFeature(ogrf, tsField));
        OGRFeature::DestroyFeature(ogrf);
    }
    return features;
}

std::string GeoPackageReader::detectTimestampField(const std::string& layerName) const {
    OGRLayer* layer = dataset->GetLayerByName(layerName.c_str());
    if (!layer) return "";
    
    OGRFeatureDefn* defn = layer->GetLayerDefn();
    
    std::vector<std::string> timeFields = {"t", "time", "timestamp", "date", "datetime", "epoch"};
    
    for (int i = 0; i < defn->GetFieldCount(); i++) {
        std::string fieldName = defn->GetFieldDefn(i)->GetNameRef();
        std::string lowerName = fieldName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        for (const auto& timeField : timeFields) {
            if (lowerName == timeField) {
                return fieldName;
            }
        }
    }
    
    return "";
}
GeoPackageReader::Feature GeoPackageReader::convertOGRFeature(OGRFeature* ogrf, const std::string& tsField) const {
    // Convertit un OGRFeature en structure interne
    Feature f;
    f.fid = ogrf->GetFID();
    f.timestamp = 0.0;

    if (!tsField.empty()) {
        int idx = ogrf->GetFieldIndex(tsField.c_str());
        if (idx >= 0) {
            auto type = ogrf->GetFieldDefnRef(idx)->GetType();
            if (type == OFTReal) f.timestamp = ogrf->GetFieldAsDouble(idx);
            else if (type == OFTInteger || type == OFTInteger64) 
                f.timestamp = ogrf->GetFieldAsInteger64(idx);
            else f.timestamp = parseTimestamp(ogrf->GetFieldAsString(idx));
        }
    }

    f.geometry = extractGeometry4D(ogrf, f.timestamp);
    
    OGRFeatureDefn* defn = ogrf->GetDefnRef();
    for (int i = 0; i < defn->GetFieldCount(); ++i) {
        f.attributes[defn->GetFieldDefn(i)->GetNameRef()] = ogrf->GetFieldAsString(i);
    }
    return f;
}

Geometry4D GeoPackageReader::extractGeometry4D(OGRFeature* ogrf, double timestamp) const {
    // Ajoute une valeur temporelle M aux géométries
    OGRGeometry* geom = ogrf->GetGeometryRef();
    if (!geom) return Geometry4D();

    OGRGeometry* clone = geom->clone();
    if (!clone->IsMeasured()) {
        clone->set3D(TRUE);
        clone->setMeasured(TRUE);
    }

    auto type = wkbFlatten(clone->getGeometryType());
    if (type == wkbPoint) {
        clone->toPoint()->setM(timestamp);
    } else if (type == wkbLineString) {
        auto* line = clone->toLineString();
        for (int i = 0; i < line->getNumPoints(); ++i) 
            line->setM(i, timestamp);
    }

    return Geometry4D(clone);
}

double GeoPackageReader::parseTimestamp(const std::string& str) const {
    // Convertit une date en timestamp
    struct tm tm = {};
    std::istringstream ss(str);
    
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) { 
        ss.clear(); 
        ss.str(str); 
        ss >> std::get_time(&tm, "%Y-%m-%d"); 
    }
    
    return ss.fail() ? 0.0 : static_cast<double>(mktime(&tm));
}



bool GeoPackageReader::isRasterLayer(const std::string& layerName) const {
    if (!isOpen) return false;
    
    // Try opening as raster with GPKG prefix
    std::string tablePath = "GPKG:" + filePath + ":" + layerName;
    GDALDataset* rasterDS = (GDALDataset*)GDALOpen(tablePath.c_str(), GA_ReadOnly);
    
    bool isRaster = false;
    if (rasterDS) {
        isRaster = (rasterDS->GetRasterCount() > 0);
        GDALClose(rasterDS);
    }
    
    return isRaster;
}
// Exrait les métadonnées raster d'une couche

GeoPackageReader::RasterMetadata 
GeoPackageReader::extractRasterMetadata(const std::string& layerName) const {
    RasterMetadata m{"", "", 0, 0, 0, 0.0, {0,0,0,0,0,0}, ""};
    if (!isOpen) return m;
    
    std::string tablePath = "GPKG:" + filePath + ":" + layerName;
    GDALDataset* rasterDS = (GDALDataset*)GDALOpen(tablePath.c_str(), GA_ReadOnly);
    
    if (!rasterDS) return m;
    
    m.name = layerName;
    m.width = rasterDS->GetRasterXSize();
    m.height = rasterDS->GetRasterYSize();
    m.layerPath = tablePath;
    
    rasterDS->GetGeoTransform(m.geoTransform);
    
    if (auto* srs = rasterDS->GetSpatialRef()) {
        if (auto* code = srs->GetAuthorityCode(nullptr)) {
            m.srid = std::atoi(code);
            m.crs = std::string(srs->GetAuthorityName(nullptr)) + ":" + code;
        }
    }
    
    const char* epochStr = rasterDS->GetMetadataItem("COORDINATE_EPOCH");
    if (epochStr) {
        m.referenceEpoch = std::stod(epochStr);
    }
    
    GDALClose(rasterDS);
    return m;
}

Geometry4D GeoPackageReader::extractRasterExtent(
    const std::string& layerName, double timestamp) const {
    
    std::string tablePath = "GPKG:" + filePath + ":" + layerName;
    GDALDataset* rasterDS = (GDALDataset*)GDALOpen(tablePath.c_str(), GA_ReadOnly);
    
    if (!rasterDS) return Geometry4D();
    
    double gt[6];
    rasterDS->GetGeoTransform(gt);
    
    int width = rasterDS->GetRasterXSize();
    int height = rasterDS->GetRasterYSize();
    
    double minX = gt[0];
    double maxY = gt[3];
    double maxX = gt[0] + width * gt[1];
    double minY = gt[3] + height * gt[5];
    
    OGRPolygon* poly = new OGRPolygon();
    OGRLinearRing* ring = new OGRLinearRing();
    
    ring->addPoint(minX, minY, 0);
    ring->addPoint(maxX, minY, 0);
    ring->addPoint(maxX, maxY, 0);
    ring->addPoint(minX, maxY, 0);
    ring->addPoint(minX, minY, 0);
    
    poly->addRing(ring);
    poly->setMeasured(TRUE);
    
    for (int i = 0; i < ring->getNumPoints(); i++) {
        ring->setM(i, timestamp);
    }
    
    GDALClose(rasterDS);
    return Geometry4D(poly);
}

bool GeoPackageReader::addTemporalField(const std::string& layerName, const std::string& fieldName, double defaultValue) {
    OGRLayer* layer = dataset->GetLayerByName(layerName.c_str());
    if (!layer) return false;
    
    // Create field
    OGRFieldDefn fieldDefn(fieldName.c_str(), OFTReal);
    if (layer->CreateField(&fieldDefn) != OGRERR_NONE) {
        return false;
    }
    
    // Set default value for all features
    layer->ResetReading();
    OGRFeature* feature;
    while ((feature = layer->GetNextFeature()) != nullptr) {
        feature->SetField(fieldName.c_str(), defaultValue);
        layer->SetFeature(feature);
        OGRFeature::DestroyFeature(feature);
    }
    
    return true;
}