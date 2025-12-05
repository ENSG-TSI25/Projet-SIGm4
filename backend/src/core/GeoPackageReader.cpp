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
    // Ouverture du GeoPackage
    if (isOpen) return true;
    dataset = (GDALDataset*)GDALOpenEx(filePath.c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY, nullptr, nullptr, nullptr);
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
    // Cherche un champ temporel
    if (!isOpen) return "";
    OGRLayer* layer = dataset->GetLayerByName(layerName.c_str());
    if (!layer) return "";

    OGRFeatureDefn* defn = layer->GetLayerDefn();
    std::vector<std::string> keywords = {"time", "timestamp", "date", "epoch"};
    
    for (int i = 0; i < defn->GetFieldCount(); ++i) {
        std::string name = defn->GetFieldDefn(i)->GetNameRef();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        for (const auto& kw : keywords) {
            if (name.find(kw) != std::string::npos) 
                return defn->GetFieldDefn(i)->GetNameRef();
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
