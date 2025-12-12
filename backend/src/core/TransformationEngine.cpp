#include <core/TransformationEngine.hpp>
#include <vector>
#include <iostream>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_spatialref.h>



// VectorLayer* TransformationEngine::transformLayerAtEpoch(VectorLayer& inputLayer, const std::string& epsg_dst){
//     // Transformation of a layer at a given epoch to a target geodetic EPSG code
//     std::vector<std::shared_ptr<Geometry4D>> vectorGeom = inputLayer.getGeometries();
    
//     std::string epsg_src = inputLayer.getCrs();
//     std::string epsg_src_clean = epsg_src.substr(epsg_src.find(":")+1);

//     // Checks the coordinate format
//     const std::string format = inputLayer.getCoordsType();
//     std::cout<<"Type de coordonnées : " << format << "\n";

//     // If the input coordinates are not geodetic, convert them to geodetic first
//     if (format == "geocentric"){
//         // Projection from geocentric/cartesian to geodetic
//         for (auto &g : vectorGeom){
//             OGRGeometry* geom = g->getGeometry();
//             OGRPoint* point = geom->toPoint();

//             const double t_init = g->getT();
//             const double x_init = point->getX();
//             const double y_init = point->getY();
//             const double z_init = point->getZ();

//             Result r = GeodeticTransformer::geocentricToGeodetic(x_init,y_init,z_init,t_init);
//             double lon = r.x;
//             double lat = r.y;
//             double h = r.z;
//             double t_out = r.t;
            
//             g->setT(t_out);
//             point->setX(lon);
//             point->setY(lat);
//             point->setZ(h);
//         }
//     }
//     else if (format == "projected"){
//         // Projection from projected to geodetic
//         for (auto &g : vectorGeom){
//             OGRGeometry* geom = g->getGeometry();
//             OGRPoint* point = geom->toPoint();

//             const double t_init = g->getT();
//             const double x_init = point->getX();
//             const double y_init = point->getY();
//             const double z_init = point->getZ();
//             Result r = GeodeticTransformer::projectedToGeodetic(x_init,y_init,z_init,t_init, epsg_src, "EPSG:4326"); // Temporary hardcoding to WGS84
//             double lon = r.x;
//             double lat = r.y;
//             double h = r.z;
//             double t_out = r.t;
            
//             g->setT(t_out);
//             point->setX(lon);
//             point->setY(lat);
//             point->setZ(h);
//         }
//         epsg_src = "EPSG:4326"; // Update CRS to geodetic after conversion
//         epsg_src_clean = "4326";
//     }
//     else if (format != "geodetic"){
//         std::cout<<format<<" is an unknown coordinate format."<<"\n";
//         return nullptr;
//     }

//     // Datum transformation if needed
//     if (epsg_src !=  "EPSG:"+epsg_dst) {
//         std::cout << "Datum transformation: " << epsg_src << " -> EPSG:" << epsg_dst << "\n";
        
//         for (auto &g : vectorGeom){
//             OGRGeometry* geom = g->getGeometry();
//             OGRPoint* point = geom->toPoint();

//             // Récupération des coordonnées et du temps
//             const double t_init = g->getT();
//             const double x_init = point->getX();
//             const double y_init = point->getY();
//             const double z_init = point->getZ();
//             std::cout<<"Coordonnées avant projection : "<< x_init << " " << y_init << " " << z_init << " " << t_init << "\n";
//             std::cout<<"EPSG source : "<< epsg_src_clean << ", EPSG destination : "<< epsg_dst << "\n";
//             Result r = GeodeticTransformer::transformGeodeticAtEpoch(x_init,y_init,z_init,t_init, epsg_src_clean, epsg_dst);
//             double x_out = r.x;
//             double y_out = r.y;
//             double z_out = r.z;
//             double t_out = r.t;
//             g->setT(t_out);
//             point->setX(x_out);
//             point->setY(y_out);
//             point->setZ(z_out);

//             std::cout<<"Coordonnées après projection : "<< x_out << " " << y_out << " " << z_out << " " << t_out << "\n";
//         }
//     }

//     inputLayer.setCrs("EPSG:"+epsg_dst);
//     return &inputLayer;
// }

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