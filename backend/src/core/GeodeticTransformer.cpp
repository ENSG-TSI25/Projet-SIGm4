#include "core/GeodeticTransformer.hpp"

#include <proj/io.hpp>
#include <proj/crs.hpp>
#include <proj/coordinateoperation.hpp>
#include <proj/util.hpp>

#include <stdexcept>
#include <memory>
#include <sstream>
#include <iostream>

using namespace NS_PROJ::io;
using namespace NS_PROJ::crs;
using namespace NS_PROJ::operation;
using namespace NS_PROJ::util;


GeodeticTransformer::GeodeticTransformer() {
    // Create a dedicated PROJ context for all transformations.
    ctx_ = proj_context_create();
}

GeodeticTransformer::~GeodeticTransformer() {
    // Destroy the PROJ context.
    proj_context_destroy(ctx_);
}

// ----------------------------------------------------
// Detect format of CRS from EPSG code
// ----------------------------------------------------
std::string GeodeticTransformer::getCRSFormat(int epsg_code) {
    // Geodetic 3D
    const auto& reg3d = GeodeticTransformer::getRegistry3D();
    if (reg3d.find(epsg_code) != reg3d.end()) {
        return "geodetic";
    }

    // Geodetic 2D
    const auto& reg2d = GeodeticTransformer::getRegistry2D();
    if (reg2d.find(epsg_code) != reg2d.end()) {
        return "geodetic";
    }

    // Projected
    const auto& regProj = GeodeticTransformer::getRegistryProjected();
    if (regProj.find(epsg_code) != regProj.end()) {
        return "projected";
    }

    return "unknown";
}

// ============================================================================
// 1. TRANSFORMATION GÉNÉRIQUE : MÈTRES -> MÈTRES
// (Geocentric <-> Geocentric, Projected <-> Projected, Geocentric <-> Projected)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::transformLinearAtEpoch(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, epsg_src.c_str(), epsg_dst.c_str(), NULL);
    if (!P) throw std::runtime_error("Transform failed: " + epsg_src + " -> " + epsg_dst);

    PJ_COORD in;
    in.xyzt.x = x; in.xyzt.y = y; in.xyzt.z = z; in.xyzt.t = t_epoch;
    
    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return { out.xyzt.x, out.xyzt.y, out.xyzt.z, out.xyzt.t };
}

// ============================================================================
// 2. TRANSFORMATION GÉNÉRIQUE : DEGRÉS -> DEGRÉS
// (Geodetic <-> Geodetic)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::transformGeodeticAtEpoch(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, epsg_src.c_str(), epsg_dst.c_str(), NULL);
    if (!P) throw std::runtime_error("Transform failed: " + epsg_src + " -> " + epsg_dst);

    PJ_COORD in;
    // INPUT : On inverse Lon(x)/Lat(y) -> Lat(x)/Lon(y) pour PROJ
    in.xyzt.x = y; 
    in.xyzt.y = x; 
    in.xyzt.z = z; in.xyzt.t = t_epoch;
    
    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    // OUTPUT : On inverse Lat(x)/Lon(y) -> Lon(x)/Lat(y) pour l'utilisateur
    return { out.xyzt.y, out.xyzt.x, out.xyzt.z, out.xyzt.t };
}

// ============================================================================
// 3. GEOCENTRIC (XYZ) -> GEODETIC (Lon, Lat)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::geocentricToGeodetic(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, epsg_src.c_str(), epsg_dst.c_str(), NULL);
    if (!P) throw std::runtime_error("GeoC -> GeoD failed");

    PJ_COORD in;
    in.xyzt.x = x; in.xyzt.y = y; in.xyzt.z = z; in.xyzt.t = t_epoch;

    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    // OUTPUT : PROJ renvoie Lat(x), Lon(y). On inverse.
    return { out.xyzt.y, out.xyzt.x, out.xyzt.z, out.xyzt.t };
}

// ============================================================================
// 4. GEODETIC (Lon, Lat) -> GEOCENTRIC (XYZ)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::geodeticToGeocentric(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, epsg_src.c_str(), epsg_dst.c_str(), NULL);
    if (!P) throw std::runtime_error("GeoD -> GeoC failed");

    PJ_COORD in;
    // INPUT : On inverse Lon(x)/Lat(y) -> Lat(x)/Lon(y)
    in.xyzt.x = y; 
    in.xyzt.y = x; 
    in.xyzt.z = z; in.xyzt.t = t_epoch;

    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return { out.xyzt.x, out.xyzt.y, out.xyzt.z, out.xyzt.t };
}

// ============================================================================
// 5. PROJECTED (E, N) -> GEODETIC (Lon, Lat)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::projectedToGeodetic(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, epsg_src.c_str(), epsg_dst.c_str(), NULL);
    if (!P) throw std::runtime_error("Proj -> GeoD failed");

    PJ_COORD in;
    in.xyzt.x = x; in.xyzt.y = y; in.xyzt.z = z; in.xyzt.t = t_epoch;

    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    // OUTPUT : PROJ renvoie Lat(x), Lon(y). On inverse.
    return { out.xyzt.y, out.xyzt.x, out.xyzt.z, out.xyzt.t };
}

// ============================================================================
// 6. GEODETIC (Lon, Lat) -> PROJECTED (E, N)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::geodeticToProjected(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, epsg_src.c_str(), epsg_dst.c_str(), NULL);
    if (!P) throw std::runtime_error("GeoD -> Proj failed");

    PJ_COORD in;
    // INPUT : On inverse Lon(x)/Lat(y) -> Lat(x)/Lon(y)
    in.xyzt.x = y;
    in.xyzt.y = x;
    in.xyzt.z = z; in.xyzt.t = t_epoch;

    PJ_COORD out = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return { out.xyzt.x, out.xyzt.y, out.xyzt.z, out.xyzt.t };
}

// ============================================================================
// 7. GEOCENTRIC (XYZ) -> PROJECTED (E, N)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::geocentricToProjected(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    // C'est du mètre vers mètre, donc comme transformLinearAtEpoch
    return transformLinearAtEpoch(x, y, z, t_epoch, epsg_src, epsg_dst);
}

// ============================================================================
// 8. PROJECTED (E, N) -> GEOCENTRIC (XYZ)
// ============================================================================
GeodeticTransformer::Result GeodeticTransformer::projectedToGeocentric(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src, const std::string& epsg_dst)
{
    // C'est du mètre vers mètre, donc comme transformLinearAtEpoch
    return transformLinearAtEpoch(x, y, z, t_epoch, epsg_src, epsg_dst);
}

// ----------------------------------------------------
// 9. applyDefModel - GEODETIC (lon, lat, h)
// ----------------------------------------------------
GeodeticTransformer::Result GeodeticTransformer::applyDefModelGeodetic(
    double lon_deg, double lat_deg, double h, double t_epoch,
    const std::string& json_model_path,
    bool inverse)
{
    std::string proj_str = "+proj=defmodel +model=" + json_model_path;
    if (inverse) proj_str += " +inv";

    PJ* P = proj_create(ctx_, proj_str.c_str());
    if (!P)
        throw std::runtime_error("Failed to load defmodel: " + json_model_path);

    PJ_COORD in;
    in.lpzt.lam = proj_torad(lon_deg);
    in.lpzt.phi = proj_torad(lat_deg);
    in.lpzt.z   = h;
    in.lpzt.t   = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return {
        proj_todeg(r.lpzt.lam),
        proj_todeg(r.lpzt.phi),
        r.lpzt.z,
        r.lpzt.t
    };
}

// ----------------------------------------------------
// 10. applyDefModel - GEOCENTRIC (X, Y, Z)
// ----------------------------------------------------
GeodeticTransformer::Result GeodeticTransformer::applyDefModelGeocentric(
    double X, double Y, double Z, double t_epoch,
    const std::string& json_model_path,
    bool inverse)
{
    std::string proj_str = "+proj=defmodel +model=" + json_model_path;
    if (inverse) proj_str += " +inv";

    PJ* P = proj_create(ctx_, proj_str.c_str());
    if (!P)
        throw std::runtime_error("Failed to load defmodel: " + json_model_path);

    PJ_COORD in;
    in.xyzt.x = X;
    in.xyzt.y = Y;
    in.xyzt.z = Z;
    in.xyzt.t = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return { r.xyzt.x,
             r.xyzt.y,
             r.xyzt.z,
             r.xyzt.t };
}

// ----------------------------------------------------
// 11. applyDefModel - PROJECTED (E, N, H)
// ----------------------------------------------------
GeodeticTransformer::Result GeodeticTransformer::applyDefModelProjected(
    double E, double N, double H, double t_epoch,
    const std::string& json_model_path,
    bool inverse)
{
    std::string proj_str = "+proj=defmodel +model=" + json_model_path;
    if (inverse) proj_str += " +inv";

    PJ* P = proj_create(ctx_, proj_str.c_str());
    if (!P)
        throw std::runtime_error("Failed to load defmodel: " + json_model_path);

    PJ_COORD in;
    in.xyzt.x = E;    // Est
    in.xyzt.y = N;    // Nord
    in.xyzt.z = H;    // Altitude
    in.xyzt.t = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return { r.xyzt.x,
             r.xyzt.y,
             r.xyzt.z,
             r.xyzt.t };
}




// // -------------------------------
// // 3. GRID-BASED DEFORMATION MODEL
// // -------------------------------

// /**
//  * @brief Apply a grid-based spatio-temporal deformation (XYZ displacement grid).
//  */
// GeodeticTransformer::Result
// GeodeticTransformer::applyGridDeformation(
//     double lon_deg, double lat_deg, double h,
//     double t_epoch,
//     const std::string& grid_path,
//     double ref_epoch)
// {
//     // 1) Geodetic (Lon, Lat) -> Cartesian (XYZ)
//     PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
//     if (!P_cart) throw std::runtime_error("Failed to create PROJ cart converter");

//     PJ_COORD in_geo;
//     in_geo.lpzt.lam = proj_torad(lon_deg);
//     in_geo.lpzt.phi = proj_torad(lat_deg);
//     in_geo.lpzt.z   = h;
//     in_geo.lpzt.t   = t_epoch;

//     PJ_COORD cart = proj_trans(P_cart, PJ_FWD, in_geo);
//     proj_destroy(P_cart);

//     // 2) Apply deformation grid in Cartesian
//     std::ostringstream ss2;
//     ss2 << "+proj=deformation +t_epoch=" << ref_epoch
//         << " +grids=" << grid_path;

//     PJ* P_def = proj_create(ctx_, ss2.str().c_str());
//     if (!P_def) throw std::runtime_error("Failed to create PROJ deformation transformer");

//     PJ_COORD in_cart2 = cart; // Copy
//     PJ_COORD cart2 = proj_trans(P_def, PJ_FWD, in_cart2);
//     proj_destroy(P_def);

//     // 3) Cartesian (XYZ) -> Geodetic (Lon, Lat)
//     // We recreate the object to be safe (or could reuse if kept alive)
//     PJ* P_cart_inv = proj_create(ctx_, "+proj=cart +ellps=GRS80"); 
    
//     // Note: Applying PJ_INV on a "+proj=cart" definition
//     PJ_COORD out_geo = proj_trans(P_cart_inv, PJ_INV, cart2);
//     proj_destroy(P_cart_inv);

//     return { proj_todeg(out_geo.lpzt.lam),
//              proj_todeg(out_geo.lpzt.phi),
//              out_geo.lpzt.z,
//              out_geo.lpzt.t };
// };

// // ----------------------------------------------------
// // 4. Projection from Geocentric/Cartesian to Geodetic
// // ----------------------------------------------------

// /**
//  * @brief Convert Geocentric (XYZ) coordinates to Geodetic (Lon, Lat, H).
//  *
//  * @param x        Geocentric X (meters).
//  * @param y        Geocentric Y (meters).
//  * @param z        Geocentric Z (meters).
//  * @param t_epoch  Observation epoch.
//  * @return Geodetic coordinates { x=Lon(deg), y=Lat(deg), z=Height(m), t }.
//  */
// GeodeticTransformer::Result
// GeodeticTransformer::geocentricToGeodetic(
//     double x, double y, double z, double t_epoch)
// {
//     // "+proj=cart" defines transformation Geodetic -> Cartesian.
//     // So for Cartesian -> Geodetic, we will use PJ_INV later.
//     PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
//     if (!P_cart) {
//         throw std::runtime_error("Failed to create PROJ cart converter");
//     }

//     PJ_COORD in_cart;
//     in_cart.xyzt.x = x;
//     in_cart.xyzt.y = y;
//     in_cart.xyzt.z = z;
//     in_cart.xyzt.t = t_epoch;

//     // Inverse transformation: XYZ -> Lon,Lat
//     PJ_COORD out_geo = proj_trans(P_cart, PJ_INV, in_cart);
//     proj_destroy(P_cart);

//     // PROJ pipeline output for +proj=cart is (Lon, Lat) in radians.
//     // out_geo.lpzt.lam corresponds to Longitude.
//     // out_geo.lpzt.phi corresponds to Latitude.
//     return { proj_todeg(out_geo.lpzt.lam), // x = Lon
//              proj_todeg(out_geo.lpzt.phi), // y = Lat
//              out_geo.lpzt.z,
//              out_geo.lpzt.t };
// };


// // ----------------------------------------------------
// // 5. Projection from Geodetic to Geocentric/Cartesian
// // ----------------------------------------------------

// /**
//  * @brief Convert Geodetic (Lon, Lat, H) coordinates to Geocentric (XYZ).
//  *
//  * @param x        Longitude (degrees).
//  * @param y        Latitude (degrees).
//  * @param z        Ellipsoidal Height (meters).
//  * @param t_epoch  Observation epoch.
//  * @return Cartesian coordinates { x=X, y=Y, z=Z, t }.
//  */
// GeodeticTransformer::Result
// GeodeticTransformer::geodeticToGeocentric(
//     double x, double y, double z, double t_epoch)
// {
//     PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
//     if (!P_cart) {
//         throw std::runtime_error("Failed to create PROJ cart converter");
//     }

//     PJ_COORD in_geo;
//     // Input for +proj=cart is (Lon, Lat) in Radians.
//     // x = Longitude, y = Latitude
//     in_geo.lpzt.lam = proj_torad(x); 
//     in_geo.lpzt.phi = proj_torad(y);
//     in_geo.lpzt.z = z;
//     in_geo.lpzt.t = t_epoch;

//     // Forward transformation: Lon,Lat -> XYZ
//     PJ_COORD out_cart = proj_trans(P_cart, PJ_FWD, in_geo);

//     proj_destroy(P_cart);

//     return { out_cart.xyzt.x,
//              out_cart.xyzt.y,
//              out_cart.xyzt.z,
//              out_cart.xyzt.t };
// };

// // ----------------------------------------------------
// // 6. Projection from Projected to Geodetic
// // ----------------------------------------------------

// /**
//  * @brief Convert Projected coordinates (Easting, Northing) to Geodetic (Lon, Lat).
//  *
//  * @param x        Easting (meters).
//  * @param y        Northing (meters).
//  * @param z        Height (meters).
//  * @param t_epoch  Observation epoch.
//  * @param epsg_src Source Projected CRS (e.g. "EPSG:2154").
//  * @param epsg_dst Target Geodetic CRS (e.g. "EPSG:4326").
//  * @return Geodetic coordinates { x=Lon, y=Lat, z=H, t }.
//  */
// GeodeticTransformer::Result 
// GeodeticTransformer::projectedToGeodetic(
//     double x, double y, double z, double t_epoch, 
//     const std::string& epsg_src,
//     const std::string& epsg_dst)
// {
//     // Create pipeline (e.g. Lambert -> Lat/Lon)
//     PJ* P = proj_create_crs_to_crs(ctx_, 
//                                    epsg_src.c_str(), 
//                                    epsg_dst.c_str(),
//                                    NULL);
    
//     if (!P) throw std::runtime_error("Failed to create CRS transformer");
    
//     PJ_COORD in;
//     in.xyzt.x = x; // Easting
//     in.xyzt.y = y; // Northing
//     in.xyzt.z = z;
//     in.xyzt.t = t_epoch;

//     PJ_COORD out_geo = proj_trans(P, PJ_FWD, in);
//     proj_destroy(P);


    
//     return { out_geo.xyzt.y,  // Longitude
//              out_geo.xyzt.x,  // Latitude
//              out_geo.xyzt.z, 
//              out_geo.xyzt.t };
// }


// // ----------------------------------------------------
// // 7. Projection from Geodetic to Projected
// // ----------------------------------------------------

// /**
//  * @brief Convert Geodetic (Lon, Lat) coordinates to Projected (Easting, Northing).
//  *
//  * @param x        Longitude (degrees).
//  * @param y        Latitude (degrees).
//  * @param z        Height (meters).
//  * @param t_epoch  Observation epoch.
//  * @param epsg_src Source Geodetic CRS (e.g. "EPSG:4326").
//  * @param epsg_dst Target Projected CRS (e.g. "EPSG:2154").
//  * @return Projected coordinates { x=Easting, y=Northing, z=H, t }.
//  */
// GeodeticTransformer::Result 
// GeodeticTransformer::geodeticToProjected(
//     double x, double y, double z, double t_epoch, 
//     const std::string& epsg_src,
//     const std::string& epsg_dst)
// {
//     PJ* P = proj_create_crs_to_crs(ctx_, 
//                                    epsg_src.c_str(), 
//                                    epsg_dst.c_str(),
//                                    NULL);
    
//     if (!P) {
//         throw std::runtime_error("Failed to create CRS transformer (Geo -> Projected)");
//     }
    
//     PJ_COORD in;

//     in.xyzt.x = y; // Latitude 
//     in.xyzt.y = x; // Longitude
    
//     in.xyzt.z = z;
//     in.xyzt.t = t_epoch;

//     PJ_COORD out_proj = proj_trans(P, PJ_FWD, in);
//     proj_destroy(P);
    
//     return { out_proj.xyzt.x, // Easting
//              out_proj.xyzt.y, //Northing
//              out_proj.xyzt.z, 
//              out_proj.xyzt.t };
// }

// // ----------------------------------------------------
// // 8. Projection from Projected to Geocentric/Cartesian
// // ----------------------------------------------------

// /**
//  * @brief Convert Projected coordinates (Easting, Northing, H) to Geocentric (XYZ).
//  *
//  * @param x        Easting (meters).
//  * @param y        Northing (meters).
//  * @param z        Height (meters).
//  * @param t_epoch  Observation epoch.
//  * @param epsg_src Source Projected CRS (e.g. "EPSG:2154").
//  * @param epsg_dst Target Geocentric CRS (e.g. "EPSG:4978").
//  * @return Geocentric coordinates { x=X, y=Y, z=Z, t }.
//  */
// GeodeticTransformer::Result 
// GeodeticTransformer::projectedToGeocentric(
//     double x, double y, double z, double t_epoch, 
//     const std::string& epsg_src,
//     const std::string& epsg_dst)
// {
//     // Create pipeline (e.g. Lambert -> XYZ ECEF)
//     PJ* P = proj_create_crs_to_crs(ctx_, 
//                                    epsg_src.c_str(), 
//                                    epsg_dst.c_str(),
//                                    NULL);
    
//     if (!P) {
//         throw std::runtime_error("Failed to create CRS transformer (Projected -> Geocentric)");
//     }
    
//     PJ_COORD in;
//     in.xyzt.x = x; // Easting
//     in.xyzt.y = y; // Northing
//     in.xyzt.z = z; // Height
//     in.xyzt.t = t_epoch;

//     // Transformation (Forward)
//     PJ_COORD out_cart = proj_trans(P, PJ_FWD, in);
//     proj_destroy(P);

//     // No unit conversion needed (Meters -> Meters)
//     return { out_cart.xyzt.x, 
//              out_cart.xyzt.y, 
//              out_cart.xyzt.z, 
//              out_cart.xyzt.t };
// }


// // ----------------------------------------------------
// // 9. Projection from Geocentric/Cartesian to Projected
// // ----------------------------------------------------

// /**
//  * @brief Convert Geocentric (XYZ) coordinates to Projected (Easting, Northing).
//  *
//  * @param x        Geocentric X (meters).
//  * @param y        Geocentric Y (meters).
//  * @param z        Geocentric Z (meters).
//  * @param t_epoch  Observation epoch.
//  * @param epsg_src Source Geocentric CRS (e.g. "EPSG:4978").
//  * @param epsg_dst Target Projected CRS (e.g. "EPSG:2154").
//  * @return Projected coordinates { x=Easting, y=Northing, z=H, t }.
//  */
// GeodeticTransformer::Result 
// GeodeticTransformer::geocentricToProjected(
//     double x, double y, double z, double t_epoch, 
//     const std::string& epsg_src,
//     const std::string& epsg_dst)
// {
//     // Create pipeline (e.g. XYZ ECEF -> Lambert)
//     PJ* P = proj_create_crs_to_crs(ctx_, 
//                                    epsg_src.c_str(), 
//                                    epsg_dst.c_str(),
//                                    NULL);
    
//     if (!P) {
//         throw std::runtime_error("Failed to create CRS transformer (Geocentric -> Projected)");
//     }
    
//     PJ_COORD in;
//     in.xyzt.x = x; // Geocentric X
//     in.xyzt.y = y; // Geocentric Y
//     in.xyzt.z = z; // Geocentric Z
//     in.xyzt.t = t_epoch;

//     // Transformation (Forward)
//     PJ_COORD out_proj = proj_trans(P, PJ_FWD, in);
//     proj_destroy(P);
    
//     // No unit conversion needed (Meters -> Meters)
//     return { out_proj.xyzt.x,  // Easting
//              out_proj.xyzt.y,  // Northing
//              out_proj.xyzt.z,  // Height
//              out_proj.xyzt.t };
// }

// // ---------------------------------------------------------------------------------------
// // 8. GENERIC LINEAR TRANSFORMATION (Projected <-> Projected OR Geocentric <-> Geocentric)
// // ---------------------------------------------------------------------------------------

// /**
//  * @brief Transform coordinates between two CRS that use linear units (Meters, Feet).
//  * * This function works for:
//  * - Projected -> Projected (e.g. Lambert-93 -> UTM)
//  * - Geocentric -> Geocentric (e.g. ITRF XYZ -> ETRF XYZ)
//  * - Projected <-> Geocentric (e.g. Lambert -> XYZ)
//  *
//  * @param x         First axis value (Easting or Geocentric X) in meters.
//  * @param y         Second axis value (Northing or Geocentric Y) in meters.
//  * @param z         Third axis value (Height or Geocentric Z) in meters.
//  * @param t_epoch   Observation epoch (decimal year).
//  * @param epsg_src  EPSG code of the source CRS.
//  * @param epsg_dst  EPSG code of the target CRS.
//  *
//  * @return Transformed coordinates (x, y, z, t) in linear units (usually meters).
//  */
// GeodeticTransformer::Result
// GeodeticTransformer::transformLinearAtEpoch(
//     double x, double y, double z, double t_epoch,
//     const std::string& epsg_src,
//     const std::string& epsg_dst)
// {
//     // 1. Initialisation
//     auto db = DatabaseContext::create();
//     auto af = AuthorityFactory::create(db, "EPSG");

//     // 2. Création des CRS
//     auto source = af->createCoordinateReferenceSystem(epsg_src);
//     auto target = af->createCoordinateReferenceSystem(epsg_dst);

//     // 3. Contexte
//     auto opCtx = CoordinateOperationContext::create(
//         AuthorityFactory::create(db, std::string()),
//         nullptr,
//         0.0
//     );

//     // 4. Recherche de l'opération
//     auto ops = CoordinateOperationFactory::create()->createOperations(
//         source, target, opCtx);

//     if (ops.empty())
//         throw std::runtime_error("No linear coordinate operation found between " + epsg_src + " and " + epsg_dst);

//     // 5. Transformation
//     auto tr = ops[0]->coordinateTransformer(ctx_);

//     // 6. Préparation de l'entrée (Directe, pas de conversion Radian)
//     PJ_COORD input;
//     input.xyzt.x = x; 
//     input.xyzt.y = y; 
//     input.xyzt.z = z;
//     input.xyzt.t = t_epoch;

//     PJ_COORD r = tr->transform(input);

//     // 7. Retour
//     return { r.xyzt.x,
//              r.xyzt.y,
//              r.xyzt.z,
//              r.xyzt.t };
// }
