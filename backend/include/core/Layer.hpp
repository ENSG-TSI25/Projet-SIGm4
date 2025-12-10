#pragma once
#include <string>

class Layer {
protected:
    std::string nom;
    std::string crs;
    double epoque;
    std::string coords_type;

public:
    Layer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", double epoque_ = 0.0, std::string coords_type_ = "geographic")
        : nom(nom_), crs(crs_), epoque(epoque_), coords_type(coords_type_) {}
    
    virtual ~Layer() = default;

    std::string getName() const { return nom; }
    std::string getCrs() const { return crs; }
    double getEpoch() const { return epoque; }
    std::string getCoordsType() const { return coords_type; }

    void setName(const std::string& n) { nom = n; }
    void setCrs(const std::string& c);
    void setEpoque(double e) { epoque = e; }

     bool operator==(const Layer& other) const {


        return nom == other.nom && crs == other.crs && epoque == other.epoque;


    }
    
};