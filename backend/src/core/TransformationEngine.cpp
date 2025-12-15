#include <core/TransformationEngine.hpp>
#include <vector>
#include <iostream>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_spatialref.h>

// // --------------------------------------------------------------
// // 1. TIME-DEPENDENT COORDINATE TRANSFORMATION (ITRF, ETRF, etc.)
// // --------------------------------------------------------------

VectorLayer* TransformationEngine::transformLayerAtEpoch(VectorLayer& inputLayer, const std::string& epsg_dst) 
{
    auto geometries = inputLayer.getGeometries();
    
    // 1. Fetch source and destination EPSG codes
    std::string src_code = inputLayer.getCrs();
    if (src_code.find("EPSG:") == std::string::npos) src_code = "EPSG:" + src_code;
    int src_code_int = std::stoi(src_code.substr(src_code.find(":")+1));
    
    std::string dst_code = epsg_dst;
    if (dst_code.find("EPSG:") == std::string::npos) dst_code = "EPSG:" + dst_code;
    int dst_code_int = std::stoi(dst_code.substr(dst_code.find(":")+1));

    // 2. Determinate format types
    std::string fmt_in = GeodeticTransformer::getCRSFormat(src_code_int);
    if (fmt_in == "unknown") {
        OGRSpatialReference srs; srs.SetFromUserInput(src_code.c_str());
        if (srs.IsGeocentric()) fmt_in = "geocentric";
        else if (srs.IsGeographic()) fmt_in = "geodetic";
        else fmt_in = "projected";
    }

    std::string fmt_out = GeodeticTransformer::getCRSFormat(dst_code_int);
    if (fmt_out == "unknown") {
        OGRSpatialReference srs; srs.SetFromUserInput(dst_code.c_str());
        if (srs.IsGeocentric()) fmt_out = "geocentric";
        else if (srs.IsGeographic()) fmt_out = "geodetic";
        else fmt_out = "projected";
    }

    std::cout << "TRANSFORMATION : " << src_code << " (" << fmt_in << ") -> " 
              << dst_code << " (" << fmt_out << ")\n";

    // 3. Exécution
    for (auto &g : geometries) {
        OGRGeometry* geom = g->getGeometry();
        if (!geom || wkbFlatten(geom->getGeometryType()) != wkbPoint) continue;
        OGRPoint* p = geom->toPoint(); // Use envelope

        Result r;

        // --- MATRICE DE DECISION (3 x 3 cas) ---
        
        // CAS 1 : Geocentric (XYZ) en entrée
        if (fmt_in == "geocentric") {
            if (fmt_out == "geocentric") {
                r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else if (fmt_out == "geodetic") {
                r = GeodeticTransformer::geocentricToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else { // projected
                r = GeodeticTransformer::geocentricToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            }
        }
        // CAS 2 : Geodetic (Lat/Lon) en entrée
        else if (fmt_in == "geodetic") {
            if (fmt_out == "geocentric") {
                r = GeodeticTransformer::geodeticToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else if (fmt_out == "geodetic") {
                r = GeodeticTransformer::transformGeodeticAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else { // projected
                r = GeodeticTransformer::geodeticToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            }
        }
        // CAS 3 : Projected (E/N) en entrée
        else { // projected
            if (fmt_out == "geocentric") {
                r = GeodeticTransformer::projectedToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else if (fmt_out == "geodetic") {
                r = GeodeticTransformer::projectedToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else { // projected
                r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            }
        }

        // Mise à jour du point
        g->setT(r.t);
        p->setX(r.x);
        p->setY(r.y);
        p->setZ(r.z);
    }

    // Mise à jour finale du Layer
    inputLayer.setCrs(dst_code);

    return &inputLayer;
}

// // ----------------------------------------------------
// // 2. JSON DEF-MODEL DEFORMATION (IGN, Mayotte, etc.)
// // ----------------------------------------------------