#pragma once
#include <memory>
#include <string>
#include <gdal/ogr_geometry.h>

class Geometry4D {
private:
    std::unique_ptr<OGRGeometry> geometry;
    std::string crs;

public:
    Geometry4D();
    Geometry4D(OGRGeometry* geom);
    Geometry4D(const Geometry4D& other);

    Geometry4D& operator=(const Geometry4D& other);
    OGRGeometry* getGeometry() const { return geometry.get(); }
    
    double getT() const;
    void setT(double timestamp);
    int getSRID() const;
    void setSRID(int srid); 

    std::string toEWKT() const;
    
};