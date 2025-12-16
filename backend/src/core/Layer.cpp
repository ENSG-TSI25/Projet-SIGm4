#include <core/Layer.hpp>
#include <gdal/ogr_geometry.h>
#include <iostream>


void Layer::setCrs(const std::string& c)
{
    // Modifies the CRS of the layer and updates coords_type accordingly
    crs = c;
    OGRSpatialReference srs;
    char* unitName = nullptr;

    if (srs.SetFromUserInput(c.c_str()) == OGRERR_NONE) {
        srs.GetLinearUnits(&unitName);
        if (srs.IsGeographic()) {
            coords_type = "geodetic"; // Lat/Lon (Degrees)
        } 
        else if (srs.IsProjected()) {
            coords_type = "projected"; // Lambert, UTM, etc. (Meters on map)
        } 
        else if (srs.IsGeocentric()) {
            coords_type = "geocentric"; // Cartesian XYZ (Meters from Earth's center)
        }
        else {
            std::cout << "Invalid coordinate units";
            coords_type = "unknown";
        }
    } else {
        std::cout << "Invalid CRS input";
        coords_type = "unknown";
    }

};