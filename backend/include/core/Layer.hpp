#pragma once
#include <string>

class Layer {
protected:
    std::string nom;
    std::string crs;
    double epoque;

public:
    Layer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", double epoque_ = 0.0)
        : nom(nom_), crs(crs_), epoque(epoque_) {}
    
    virtual ~Layer() = default;

    std::string getName() const { return nom; }
    std::string getCrs() const { return crs; }
    double getEpoch() const { return epoque; }

    void setName(const std::string& n) { nom = n; }
    void setCrs(const std::string& c) { crs = c; }
    void setEpoque(double e) { epoque = e; }
};