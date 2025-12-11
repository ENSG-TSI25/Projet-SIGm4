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

/**
 * @class GeodeticTransformer
 * @brief Wrapper around the PROJ API for various spatio-temporal transformations.
 *
 * A dedicated PROJ context is maintained for the lifetime of the object.
 */
GeodeticTransformer::GeodeticTransformer() {
    // Create a dedicated PROJ context for all transformations.
    ctx_ = proj_context_create();
}

GeodeticTransformer::~GeodeticTransformer() {
    // Destroy the PROJ context.
    proj_context_destroy(ctx_);
}

//
// 1. TIME-DEPENDENT COORDINATE TRANSFORMATION (ITRF, ETRF, etc.)
// --------------------------------------------------------------

/**
 * @brief Transform geodetic coordinates (lon, lat, h) between EPSG frames
 * while accounting for the observation epoch.
 *
 * @param lon       Longitude in degrees.
 * @param lat       Latitude in degrees.
 * @param h         Ellipsoidal height (meters).
 * @param t_epoch   Observation epoch (decimal year).
 * @param epsg_src  EPSG code of the source CRS (e.g., "4978").
 * @param epsg_dst  EPSG code of the target CRS (e.g., "4937").
 *
 * @return Transformed coordinates (lon°, lat°, h, t_epoch).
 */
GeodeticTransformer::Result
GeodeticTransformer::transformAtEpoch(
    double lon, double lat, double h, double t_epoch,
    const std::string& epsg_src,
    const std::string& epsg_dst)
{
    auto db = DatabaseContext::create();
    auto af = AuthorityFactory::create(db, "EPSG");

    auto source = af->createCoordinateReferenceSystem(epsg_src);
    auto target = af->createCoordinateReferenceSystem(epsg_dst);

    auto opCtx = CoordinateOperationContext::create(
        AuthorityFactory::create(db, std::string()),
        nullptr,
        0.0
    );

    auto ops = CoordinateOperationFactory::create()->createOperations(
        source, target, opCtx);

    if (ops.empty())
        throw std::runtime_error("No coordinate operation found.");

    auto tr = ops[0]->coordinateTransformer(ctx_);

    PJ_COORD input;
    input.lpzt.lam = proj_torad(lon); // Lambda = Lon
    input.lpzt.phi = proj_torad(lat); // Phi = Lat
    input.lpzt.z   = h;
    input.lpzt.t   = t_epoch;

    PJ_COORD r = tr->transform(input);

    return { proj_todeg(r.lpzt.lam),
             proj_todeg(r.lpzt.phi),
             r.lpzt.z,
             r.lpzt.t };
}

//
// 2. JSON DEF-MODEL DEFORMATION (IGN, Mayotte, etc.)
// ----------------------------------------------------

/**
 * @brief Apply a deformation model described in a JSON "defmodel" file.
 */
GeodeticTransformer::Result
GeodeticTransformer::applyDefModel(
    double lon, double lat, double h, double t_epoch,
    const std::string& json_model_path,
    bool inverse)
{
    std::string proj_str = "+proj=defmodel +model=" + json_model_path;
    if (inverse) proj_str += " +inv";

    PJ* P = proj_create(ctx_, proj_str.c_str());
    if (!P)
        throw std::runtime_error("Failed to load defmodel: " + json_model_path);

    PJ_COORD input;
    input.lpzt.lam = proj_torad(lon);
    input.lpzt.phi = proj_torad(lat);
    input.lpzt.z   = h;
    input.lpzt.t   = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, input);
    proj_destroy(P);

    return { proj_todeg(r.lpzt.lam),
             proj_todeg(r.lpzt.phi),
             r.lpzt.z,
             r.lpzt.t };
}

//
// 3. GRID-BASED DEFORMATION MODEL
// -------------------------------

/**
 * @brief Apply a grid-based spatio-temporal deformation (XYZ displacement grid).
 */
GeodeticTransformer::Result
GeodeticTransformer::applyGridDeformation(
    double lon_deg, double lat_deg, double h,
    double t_epoch,
    const std::string& grid_path,
    double ref_epoch)
{
    // 1) Geodetic (Lon, Lat) -> Cartesian (XYZ)
    PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart) throw std::runtime_error("Failed to create PROJ cart converter");

    PJ_COORD in_geo;
    in_geo.lpzt.lam = proj_torad(lon_deg);
    in_geo.lpzt.phi = proj_torad(lat_deg);
    in_geo.lpzt.z   = h;
    in_geo.lpzt.t   = t_epoch;

    PJ_COORD cart = proj_trans(P_cart, PJ_FWD, in_geo);
    proj_destroy(P_cart);

    // 2) Apply deformation grid in Cartesian
    std::ostringstream ss2;
    ss2 << "+proj=deformation +t_epoch=" << ref_epoch
        << " +grids=" << grid_path;

    PJ* P_def = proj_create(ctx_, ss2.str().c_str());
    if (!P_def) throw std::runtime_error("Failed to create PROJ deformation transformer");

    PJ_COORD in_cart2 = cart; // Copy
    PJ_COORD cart2 = proj_trans(P_def, PJ_FWD, in_cart2);
    proj_destroy(P_def);

    // 3) Cartesian (XYZ) -> Geodetic (Lon, Lat)
    // We recreate the object to be safe (or could reuse if kept alive)
    PJ* P_cart_inv = proj_create(ctx_, "+proj=cart +ellps=GRS80"); 
    
    // Note: Applying PJ_INV on a "+proj=cart" definition
    PJ_COORD out_geo = proj_trans(P_cart_inv, PJ_INV, cart2);
    proj_destroy(P_cart_inv);

    return { proj_todeg(out_geo.lpzt.lam),
             proj_todeg(out_geo.lpzt.phi),
             out_geo.lpzt.z,
             out_geo.lpzt.t };
};

// ----------------------------------------------------
// 4. Projection from Geocentric/Cartesian to Geodetic
// ----------------------------------------------------

/**
 * @brief Convert Geocentric (XYZ) coordinates to Geodetic (Lon, Lat, H).
 *
 * @param x        Geocentric X (meters).
 * @param y        Geocentric Y (meters).
 * @param z        Geocentric Z (meters).
 * @param t_epoch  Observation epoch.
 * @return Geodetic coordinates { x=Lon(deg), y=Lat(deg), z=Height(m), t }.
 */
GeodeticTransformer::Result
GeodeticTransformer::geocentricToGeodetic(
    double x, double y, double z, double t_epoch)
{
    // "+proj=cart" defines transformation Geodetic -> Cartesian.
    // So for Cartesian -> Geodetic, we will use PJ_INV later.
    PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart) {
        throw std::runtime_error("Failed to create PROJ cart converter");
    }

    PJ_COORD in_cart;
    in_cart.xyzt.x = x;
    in_cart.xyzt.y = y;
    in_cart.xyzt.z = z;
    in_cart.xyzt.t = t_epoch;

    // Inverse transformation: XYZ -> Lon,Lat
    PJ_COORD out_geo = proj_trans(P_cart, PJ_INV, in_cart);
    proj_destroy(P_cart);

    // PROJ pipeline output for +proj=cart is (Lon, Lat) in radians.
    // out_geo.lpzt.lam corresponds to Longitude.
    // out_geo.lpzt.phi corresponds to Latitude.
    return { proj_todeg(out_geo.lpzt.lam), // x = Lon
             proj_todeg(out_geo.lpzt.phi), // y = Lat
             out_geo.lpzt.z,
             out_geo.lpzt.t };
};


// ----------------------------------------------------
// 5. Projection from Geodetic to Geocentric/Cartesian
// ----------------------------------------------------

/**
 * @brief Convert Geodetic (Lon, Lat, H) coordinates to Geocentric (XYZ).
 *
 * @param x        Longitude (degrees).
 * @param y        Latitude (degrees).
 * @param z        Ellipsoidal Height (meters).
 * @param t_epoch  Observation epoch.
 * @return Cartesian coordinates { x=X, y=Y, z=Z, t }.
 */
GeodeticTransformer::Result
GeodeticTransformer::geodeticToGeocentric(
    double x, double y, double z, double t_epoch)
{
    PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart) {
        throw std::runtime_error("Failed to create PROJ cart converter");
    }

    PJ_COORD in_geo;
    // Input for +proj=cart is (Lon, Lat) in Radians.
    // x = Longitude, y = Latitude
    in_geo.lpzt.lam = proj_torad(x); 
    in_geo.lpzt.phi = proj_torad(y);
    in_geo.lpzt.z = z;
    in_geo.lpzt.t = t_epoch;

    // Forward transformation: Lon,Lat -> XYZ
    PJ_COORD out_cart = proj_trans(P_cart, PJ_FWD, in_geo);

    proj_destroy(P_cart);

    return { out_cart.xyzt.x,
             out_cart.xyzt.y,
             out_cart.xyzt.z,
             out_cart.xyzt.t };
};

// ----------------------------------------------------
// 6. Projection from Projected to Geodetic
// ----------------------------------------------------

/**
 * @brief Convert Projected coordinates (Easting, Northing) to Geodetic (Lon, Lat).
 *
 * @param x        Easting (meters).
 * @param y        Northing (meters).
 * @param z        Height (meters).
 * @param t_epoch  Observation epoch.
 * @param epsg_src Source Projected CRS (e.g. "EPSG:2154").
 * @param epsg_dst Target Geodetic CRS (e.g. "EPSG:4326").
 * @return Geodetic coordinates { x=Lon, y=Lat, z=H, t }.
 */
GeodeticTransformer::Result 
GeodeticTransformer::projectedToGeodetic(
    double x, double y, double z, double t_epoch, 
    const std::string& epsg_src,
    const std::string& epsg_dst)
{
    // Create pipeline (e.g. Lambert -> Lat/Lon)
    PJ* P = proj_create_crs_to_crs(ctx_, 
                                   epsg_src.c_str(), 
                                   epsg_dst.c_str(),
                                   NULL);
    
    if (!P) throw std::runtime_error("Failed to create CRS transformer");
    
    PJ_COORD in;
    in.xyzt.x = x; // Easting
    in.xyzt.y = y; // Northing
    in.xyzt.z = z;
    in.xyzt.t = t_epoch;

    PJ_COORD out_geo = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);


    
    return { out_geo.xyzt.y,  // Longitude
             out_geo.xyzt.x,  // Latitude
             out_geo.xyzt.z, 
             out_geo.xyzt.t };
}


// ----------------------------------------------------
// 7. Projection from Geodetic to Projected
// ----------------------------------------------------

/**
 * @brief Convert Geodetic (Lon, Lat) coordinates to Projected (Easting, Northing).
 *
 * @param x        Longitude (degrees).
 * @param y        Latitude (degrees).
 * @param z        Height (meters).
 * @param t_epoch  Observation epoch.
 * @param epsg_src Source Geodetic CRS (e.g. "EPSG:4326").
 * @param epsg_dst Target Projected CRS (e.g. "EPSG:2154").
 * @return Projected coordinates { x=Easting, y=Northing, z=H, t }.
 */
GeodeticTransformer::Result 
GeodeticTransformer::geodeticToProjected(
    double x, double y, double z, double t_epoch, 
    const std::string& epsg_src,
    const std::string& epsg_dst)
{
    PJ* P = proj_create_crs_to_crs(ctx_, 
                                   epsg_src.c_str(), 
                                   epsg_dst.c_str(),
                                   NULL);
    
    if (!P) {
        throw std::runtime_error("Failed to create CRS transformer (Geo -> Projected)");
    }
    
    PJ_COORD in;

    in.xyzt.x = y; // Latitude 
    in.xyzt.y = x; // Longitude
    
    in.xyzt.z = z;
    in.xyzt.t = t_epoch;

    PJ_COORD out_proj = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);
    
    return { out_proj.xyzt.x, // Easting
             out_proj.xyzt.y, //Northing
             out_proj.xyzt.z, 
             out_proj.xyzt.t };
}