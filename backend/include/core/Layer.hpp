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

    std::string getNom() const { return nom; }
    // Alias in English for tests
    std::string getName() const { return nom; }
    std::string getCrs() const { return crs; }
    double getEpoque() const { return epoque; }

    void setNom(const std::string& n) { nom = n; }
    void setCrs(const std::string& c) { crs = c; }
    void setEpoque(double e) { epoque = e; }
    
    bool operator==(const Layer& other) const {
        return nom == other.nom && crs == other.crs && epoque == other.epoque;
    }
};