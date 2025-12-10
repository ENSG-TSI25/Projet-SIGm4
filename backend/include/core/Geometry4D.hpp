#pragma once

#include <memory>
#include <string>
#include <gdal/ogr_geometry.h>

/**
 * @class Geometry4D
 * @brief 4D geometry with temporal dimension
 * 
 * Extends OGR geometries with time coordinate (T).
 * Used for spatio-temporal data in the project.
 */
class Geometry4D {
private:
    /// Underlying OGR geometry
    std::unique_ptr<OGRGeometry> geometry;
    
    /// Coordinate reference system
    std::string crs;

public:
    /**
     * @brief Default constructor
     */
    Geometry4D();
    
    /**
     * @brief Constructor from OGR geometry
     * @param geom Existing OGR geometry
     */
    Geometry4D(OGRGeometry* geom);
    
    /**
     * @brief Copy constructor
     */
    Geometry4D(const Geometry4D& other);

    /**
     * @brief Assignment operator
     */
    Geometry4D& operator=(const Geometry4D& other);
    
    /**
     * @brief Gets the OGR geometry
     * @return Pointer to OGR geometry
     */
    OGRGeometry* getGeometry() const { return geometry.get(); }
    
    /**
     * @brief Gets temporal coordinate T
     * @return Time value
     */
    double getT() const;
    
    /**
     * @brief Sets temporal coordinate T
     * @param timestamp Time value to set
     */
    void setT(double timestamp);
    
    /**
     * @brief Gets SRID
     * @return EPSG code
     */
    int getSRID() const;
    
    /**
     * @brief Sets SRID
     * @param srid EPSG code to set
     */
    void setSRID(int srid);

    /**
     * @brief Converts to EWKT format
     * @return EWKT string
     * 
     * Used for display and storage.
     */
    std::string toEWKT() const;
};