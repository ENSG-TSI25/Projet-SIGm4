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

    // 2. Determine format types
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

    // 3. Execution
    for (auto &g : geometries) {
        OGRGeometry* geom = g->getGeometry();
        if (!geom || wkbFlatten(geom->getGeometryType()) != wkbPoint) continue;
        OGRPoint* p = geom->toPoint(); // Use envelope

        Result r;

        // --- DECISION MATRIX (3 x 3 cases) ---
        
        // CAS 1 : Geocentric (XYZ) input
        if (fmt_in == "geocentric") {
            if (fmt_out == "geocentric") {
                r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else if (fmt_out == "geodetic") {
                r = GeodeticTransformer::geocentricToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else { // projected
                r = GeodeticTransformer::geocentricToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            }
        }
        // CAS 2 : Geodetic (Lat/Lon) input
        else if (fmt_in == "geodetic") {
            if (fmt_out == "geocentric") {
                r = GeodeticTransformer::geodeticToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else if (fmt_out == "geodetic") {
                r = GeodeticTransformer::transformGeodeticAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            } else { // projected
                r = GeodeticTransformer::geodeticToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
            }
        }
        // CAS 3 : Projected (E/N) input
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

VectorLayer* TransformationEngine::applyDefModelLayer(VectorLayer& inputLayer, const std::string& json_model_path, bool inverse)
{
    auto geometries = inputLayer.getGeometries();
    
    // 1. Fetch source EPSG code
    std::string src_code = inputLayer.getCrs();
    if (src_code.find("EPSG:") == std::string::npos) src_code = "EPSG:" + src_code;
    int src_code_int = std::stoi(src_code.substr(src_code.find(":")+1));

    // 2. Determine format type
    std::string fmt_in = GeodeticTransformer::getCRSFormat(src_code_int);
    if (fmt_in == "unknown") {
        OGRSpatialReference srs; srs.SetFromUserInput(src_code.c_str());
        if (srs.IsGeocentric()) fmt_in = "geocentric";
        else if (srs.IsGeographic()) fmt_in = "geodetic";
        else fmt_in = "projected";
    }

    // 3. Execution
    for (auto &g : geometries) {
        OGRGeometry* geom = g->getGeometry();
        if (!geom || wkbFlatten(geom->getGeometryType()) != wkbPoint) continue;
        OGRPoint* p = geom->toPoint(); // Use envelope

        Result r;
        
        // CAS 1 : Geocentric (XYZ) input
        if (fmt_in == "geocentric") {
            r = GeodeticTransformer::applyDefModelGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), json_model_path, inverse);
        }
        // CAS 2 : Geodetic (Lat/Lon) input
        else if (fmt_in == "geodetic") {
            r = GeodeticTransformer::applyDefModelGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), json_model_path, inverse);
        }
        // CAS 3 : Projected (E/N) input
        else { // projected
            r = GeodeticTransformer::applyDefModelProjected(p->getX(), p->getY(), p->getZ(), g->getT(), json_model_path, src_code_int, inverse);
        }

        // Mise à jour du point
        g->setT(r.t);
        p->setX(r.x);
        p->setY(r.y);
        p->setZ(r.z);
    }

    return &inputLayer;
}

// // -------------------------------
// // 3. GRID-BASED DEFORMATION MODEL
// // -------------------------------

VectorLayer* TransformationEngine::applyGridDeformationLayer(VectorLayer& inputLayer, const std::string& grid_path, double ref_epoch)
{
    auto geometries = inputLayer.getGeometries();
    
    // 1. Fetch source and destination EPSG codes
    std::string src_code = inputLayer.getCrs();
    if (src_code.find("EPSG:") == std::string::npos) src_code = "EPSG:" + src_code;
    int src_code_int = std::stoi(src_code.substr(src_code.find(":")+1));

    // 2. Determine format types
    std::string fmt_in = GeodeticTransformer::getCRSFormat(src_code_int);
    if (fmt_in == "unknown") {
        OGRSpatialReference srs; srs.SetFromUserInput(src_code.c_str());
        if (srs.IsGeocentric()) fmt_in = "geocentric";
        else if (srs.IsGeographic()) fmt_in = "geodetic";
        else fmt_in = "projected";
    }

    // 3. Execution
    for (auto &g : geometries) {
        OGRGeometry* geom = g->getGeometry();
        if (!geom || wkbFlatten(geom->getGeometryType()) != wkbPoint) continue;
        OGRPoint* p = geom->toPoint(); // Use envelope

        Result r;
        
        // CAS 1 : Geocentric (XYZ) input
        if (fmt_in == "geocentric") {
            r = GeodeticTransformer::applyGridDeformationGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), grid_path, ref_epoch);
        }
        // CAS 2 : Geodetic (Lat/Lon) input
        else if (fmt_in == "geodetic") {
            r = GeodeticTransformer::applyGridDeformationGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), grid_path, ref_epoch);
        }
        // CAS 3 : Projected (E/N) input
        else { // projected
            r = GeodeticTransformer::applyGridDeformationProjected(p->getX(), p->getY(), p->getZ(), g->getT(), grid_path, src_code_int, ref_epoch);
        }

        // Mise à jour du point
        g->setT(r.t);
        p->setX(r.x);
        p->setY(r.y);
        p->setZ(r.z);
    }

    return &inputLayer;
}