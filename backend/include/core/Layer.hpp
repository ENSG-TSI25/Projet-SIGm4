#pragma once
#include <string>

class Layer
{
protected:
    std::string nom;
    std::string crs;
    double epoque;
    std::string coords_type;
    std::string dataSource; 


public:
    Layer(const std::string& nom_, const std::string& crs_ = "EPSG:4326",
          double epoque_ = 0.0, std::string coords_type_ = "geographic", const std::string& dataSource_ = "")
        : nom(nom_), crs(crs_), epoque(epoque_), coords_type(coords_type_), dataSource(dataSource_) {}
    

    virtual ~Layer() = default;

    std::string getName() const { return nom; }
    std::string getCrs() const { return crs; }
    double getEpoch() const { return epoque; }
    std::string getCoordsType() const { return coords_type; }

    std::string getDataSource() const { return dataSource; }

    void setName(const std::string &n) { nom = n; }
    void setCrs(const std::string &c) { crs = c; }
    void setEpoque(double e) { epoque = e; }
    void setDataSource(const std::string& path) { dataSource = path; }

    bool operator==(const Layer& other) const {
        return nom == other.nom && crs == other.crs && 
               epoque == other.epoque && dataSource == other.dataSource;
    bool operator==(const Layer &other) const
};