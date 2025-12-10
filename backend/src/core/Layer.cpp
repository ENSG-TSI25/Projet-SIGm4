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
        if (unitName == "metre"){
            coords_type = "cartesian";  
        if (unitName == "degree"){
            coords_type = "geographic";
        }
        else {
            std::cout << "Invalid coordinate units";
            coords_type = "unknown";
        }
    }
    } else {
        std::cout << "Invalid CRS input";
        coords_type = "unknown";
    }

};