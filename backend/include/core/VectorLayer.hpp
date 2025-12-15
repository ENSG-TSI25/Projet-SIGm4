#pragma once
#include <core/Layer.hpp>
#include <core/Geometry4D.hpp>
#include <vector>
#include <memory>

class VectorLayer : public Layer
{
private:
    bool m_hasTemporalData; 
    std::string timestampField;
    std::map<int, std::map<std::string, std::string>> featureAttributes;
    std::vector<std::shared_ptr<Geometry4D>> geometries;
    std::vector<std::string> ewktStrings;
    
public:
    VectorLayer(const std::string& nom_, const std::string& crs_ = "EPSG:4326",
                double epoque_ = 0.0, std::string coords_type_ = "geodetic", const std::string& dataSource_ = "")
        : Layer(nom_, crs_, epoque_, coords_type_, dataSource_), m_hasTemporalData(false) {}

    
    void addGeometry(std::shared_ptr<Geometry4D> geom) {
        geometries.push_back(geom);
        ewktStrings.push_back(geom->toEWKT());
    }
    
    const std::vector<std::shared_ptr<Geometry4D>> &getGeometries() const {
        return geometries;
    }
    
    const std::vector<std::string> &getEWKT() const {
        return ewktStrings;
    }
    
    bool hasTemporalData() const { return m_hasTemporalData; }
    std::string getTimestampField() const { return timestampField; }
    
    void setTemporalData(bool temporal, const std::string& field) {
        m_hasTemporalData = temporal;
        timestampField = field;
    }
    
    void addFeatureAttributes(int fid, const std::map<std::string, std::string>& attrs) {
        featureAttributes[fid] = attrs;
    }
    
    std::map<std::string, std::string> getFeatureAttributes(int fid) const {
        auto it = featureAttributes.find(fid);
        return (it != featureAttributes.end()) ? it->second : std::map<std::string, std::string>();
    }
};