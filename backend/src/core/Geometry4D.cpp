#include <core/Geometry4D.hpp>
#include <sstream>

/**
 * @file Geometry4D.cpp
 * @brief Implementation of Geometry4D class
 * 
 * Contains 4D geometry operations with temporal dimension.
 */

/**
 * @brief Default constructor
 */
Geometry4D::Geometry4D() : geometry(nullptr), crs("EPSG:4326") {}

/**
 * @brief Constructor from OGR geometry
 * @param geom OGR geometry pointer
 * 
 * Enables 3D and measured dimensions if not already set.
 */
Geometry4D::Geometry4D(OGRGeometry* geom) : geometry(geom), crs("EPSG:4326") {
    if (geometry && !geometry->IsMeasured()) {
        geometry->set3D(TRUE);
        geometry->setMeasured(TRUE);
    }
}

/**
 * @brief Copy constructor
 * @param other Geometry4D to copy
 */
Geometry4D::Geometry4D(const Geometry4D& other) : crs(other.crs) {
    if (other.geometry) {
        geometry.reset(other.geometry->clone());
    }
}

/**
 * @brief Assignment operator
 * @param other Geometry4D to assign from
 * @return Reference to this object
 */
Geometry4D& Geometry4D::operator=(const Geometry4D& other) {
    if (this != &other) {
        crs = other.crs;
        geometry.reset(other.geometry ? other.geometry->clone() : nullptr);
    }
    return *this;
}

/**
 * @brief Gets temporal coordinate T
 * @return Time value from M coordinate
 */
double Geometry4D::getT() const {
    if (!geometry || !geometry->IsMeasured()) return 0.0;
    auto type = wkbFlatten(geometry->getGeometryType());
    if (type == wkbPoint) return geometry->toPoint()->getM();
    if (type == wkbLineString) return geometry->toLineString()->getM(0);
    return 0.0;
}

/**
 * @brief Sets temporal coordinate T
 * @param ts Time value to set
 */
void Geometry4D::setT(double ts) {
    if (!geometry) return;
    auto type = wkbFlatten(geometry->getGeometryType());
    if (type == wkbPoint) geometry->toPoint()->setM(ts);
    else if (type == wkbLineString) {
        auto* line = geometry->toLineString();
        for (int i = 0; i < line->getNumPoints(); ++i) line->setM(i, ts);
    }
}

/**
 * @brief Gets SRID from geometry
 * @return EPSG code or 0 if not set
 */
int Geometry4D::getSRID() const {
    if (!geometry) return 0;
    auto* srs = geometry->getSpatialReference();
    if (!srs) return 0;
    auto* code = srs->GetAuthorityCode(nullptr);
    return code ? std::atoi(code) : 0;
}

/**
 * @brief Sets SRID for geometry
 * @param srid EPSG code to set
 */
void Geometry4D::setSRID(int srid) {
    if (!geometry) return;
    
    OGRSpatialReference* srs = new OGRSpatialReference();
    srs->importFromEPSG(srid);
    geometry->assignSpatialReference(srs);
    srs->Release();
    
    crs = "EPSG:" + std::to_string(srid);
}

/**
 * @brief Converts geometry to EWKT format
 * @return EWKT string with SRID and coordinates
 */
std::string Geometry4D::toEWKT() const {
    if (!geometry) return "";
    
    std::ostringstream oss;
    
    // SRID
    int srid = getSRID();
    if (srid > 0) {
        oss << "SRID=" << srid << ";";
    }
    
    // WKT with M coordinate
    char* wkt = nullptr;
    geometry->exportToWkt(&wkt);
    oss << wkt;
    CPLFree(wkt);
    
    return oss.str();
}