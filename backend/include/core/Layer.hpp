#pragma once

#include <string>

/**
 * @class Layer
 * @brief Base class for all layers
 * 
 * Contains common properties for vector and raster layers.
 */
class Layer {
protected:
    std::string nom;
    std::string crs;
    double epoque;

public:
    /**
     * @brief Constructor
     * @param nom_ Layer name
     * @param crs_ Coordinate reference system
     * @param epoque_ Reference epoch
     */
    Layer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", double epoque_ = 0.0)
        : nom(nom_), crs(crs_), epoque(epoque_) {}
    
    virtual ~Layer() = default;

    std::string getName() const { return nom; }
    std::string getCrs() const { return crs; }
    double getEpoch() const { return epoque; }

    void setName(const std::string& n) { nom = n; }
    void setCrs(const std::string& c) { crs = c; }
    void setEpoque(double e) { epoque = e; }

    /**
     * @brief Equality operator
     * @param other Layer to compare with
     * @return true if layers are equal
     */
    bool operator==(const Layer& other) const {
        return nom == other.nom && crs == other.crs && epoque == other.epoque;
    }
};