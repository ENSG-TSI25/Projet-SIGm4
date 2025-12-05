/**
 * @file geodetictransformer.cpp
 * @brief Implements utilities for geodetic coordinate transformations using PROJ.
 *
 * This class provides:
 *   1. Time-dependent geodetic transformations between CRS (e.g., ITRF ↔ ETRF),
 *   2. Application of PROJ JSON deformation models ("defmodel"),
 *   3. Application of grid-based displacement models (e.g., seismic or tectonic grids).
 *
 * All operations support spatio-temporal coordinates (lon, lat, height, epoch).
 */

#include "geodetictransformer.hpp"

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
 *        while accounting for the observation epoch.
 *
 * PROJ will automatically select the best available transformation,
 * including time-dependent Helmert transformations when appropriate.
 *
 * @param lon       Longitude in degrees.
 * @param lat       Latitude in degrees.
 * @param h         Ellipsoidal height (meters).
 * @param t_epoch   Observation epoch (decimal year).
 * @param epsg_src  EPSG code of the source CRS (e.g., "EPSG:4978").
 * @param epsg_dst  EPSG code of the target CRS (e.g., "EPSG:4937").
 *
 * @return Transformed coordinates (lon°, lat°, h, t_epoch).
 *
 * @throws std::runtime_error if no valid transformation is found.
 */
GeodeticTransformer::Result
GeodeticTransformer::transformAtEpoch(
    double lon, double lat, double h, double t_epoch,
    const std::string& epsg_src,
    const std::string& epsg_dst)
{
    // Create database context to access EPSG entries.
    auto db = DatabaseContext::create();
    auto af = AuthorityFactory::create(db, "EPSG");

    // Load source and target CRS.
    auto source = af->createCoordinateReferenceSystem(epsg_src);
    auto target = af->createCoordinateReferenceSystem(epsg_dst);

    // Create an operation context for searching appropriate transformations.
    auto opCtx = CoordinateOperationContext::create(
        AuthorityFactory::create(db, std::string()),
        nullptr,
        0.0
    );

    // debut changements
    // // Ask PROJ for all potential transformations.
    // auto ops = CoordinateOperationFactory::create()->createOperations(
    //     source, target, opCtx);

    // if (ops.empty())
    //     throw std::runtime_error("No coordinate operation found.");

    // // Use the first available transformation.
    // auto tr = osgeo::proj::operation::CoordinateTransformer::create(ops[0], ctx_);

    // // Prepare input spatio-temporal coordinate.
    // PJ_COORD input;
    // input.lpzt.phi = proj_torad(lat);
    // input.lpzt.lam = proj_torad(lon);
    // input.lpzt.z   = h;
    // input.lpzt.t   = t_epoch;

//     // Apply transformation.
//     PJ_COORD r = tr->transform(input);

//     // Convert back to degrees.
//     return { proj_todeg(r.lpzt.lam),
//              proj_todeg(r.lpzt.phi),
//              r.lpzt.z,
//              r.lpzt.t };
// }
    // fin changements


// Ask PROJ for all potential transformations.
auto ops = CoordinateOperationFactory::create()->createOperations(
    source, target, opCtx);

if (ops.empty())
    throw std::runtime_error("No coordinate operation found.");

// Use the first available transformation - convert to PJ* directly
std::string proj_string = ops[0]->exportToPROJString(
    PROJStringFormatter::create(PROJStringFormatter::Convention::PROJ_5, db).get());

PJ* P = proj_create(ctx_, proj_string.c_str());
if (!P)
    throw std::runtime_error("Failed to create transformation from PROJ string.");

// Prepare input spatio-temporal coordinate.
PJ_COORD input;
input.lpzt.phi = proj_torad(lat);
input.lpzt.lam = proj_torad(lon);
input.lpzt.z   = h;
input.lpzt.t   = t_epoch;

// Apply transformation.
PJ_COORD r = proj_trans(P, PJ_FWD, input);

// Clean up
proj_destroy(P);

// Convert back to degrees.
return { proj_todeg(r.lpzt.lam),
         proj_todeg(r.lpzt.phi),
         r.lpzt.z,
         r.lpzt.t };
//
// 2. JSON DEF-MODEL DEFORMATION (IGN, Mayotte, etc.)
// ----------------------------------------------------

/**
 * @brief Apply a deformation model described in a JSON "defmodel" file.
 *
 * This is typically used for models published by national agencies (IGN, LINZ,...)
 * or local volcanic/seismic deformation models.
 *
 * @param lon            Longitude in degrees.
 * @param lat            Latitude in degrees.
 * @param h              Ellipsoidal height.
 * @param t_epoch        Observation epoch.
 * @param json_model_path Path to the JSON deformation model.
 * @param inverse        Apply inverse deformation if true.
 *
 * @return Deformed coordinates (lon°, lat°, h, t_epoch).
 *
 * @throws std::runtime_error if the model cannot be loaded.
 */
GeodeticTransformer::Result

GeodeticTransformer::applyDefModel(
    double lon, double lat, double h, double t_epoch,
    const std::string& json_model_path,
    bool inverse)
{
    // Build the PROJ pipeline string for the defmodel.
    std::string proj_str = "+proj=defmodel +model=" + json_model_path;
    if (inverse) proj_str += " +inv";

    // Create the PROJ transformation object.
    PJ* P = proj_create(ctx_, proj_str.c_str());
    if (!P)
        throw std::runtime_error("Failed to load defmodel: " + json_model_path);

    // Prepare input coordinate (deg → rad).
    PJ_COORD input;
    input.lpzt.phi = proj_torad(lat);
    input.lpzt.lam = proj_torad(lon);
    input.lpzt.z   = h;
    input.lpzt.t   = t_epoch;

    // Apply deformation.
    PJ_COORD r = proj_trans(P, PJ_FWD, input);

    // Clean up.
    proj_destroy(P);

    // Convert back to degrees.
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
 *
 * Typical use cases:
 *   - Nordic deformation models,
 *   - Seismic displacement models,
 *   - Tectonic velocity grid integration.
 *
 * The deformation is applied in Cartesian coordinates, then converted back to
 * geodetic coordinates.
 *
 * @param lon_deg    Longitude in degrees.
 * @param lat_deg    Latitude in degrees.
 * @param h          Ellipsoidal height.
 * @param t_epoch    Observation epoch (decimal year).
 * @param grid_path  Path to the deformation grid file.
 * @param ref_epoch  Reference epoch of the deformation grid.
 *
 * @return Deformed (lon°, lat°, h, t).
 *
 * @throws std::runtime_error if PROJ fails to create any step.
 */
GeodeticTransformer::Result
GeodeticTransformer::applyGridDeformation(
    double lon_deg, double lat_deg, double h,
    double t_epoch,
    const std::string& grid_path,
    double ref_epoch)
{
    // 1) Convert geodetic → Cartesian.
    PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart) {
        throw std::runtime_error("Failed to create PROJ cart converter");
    }

    PJ_COORD in_geo;
    in_geo.lpzt.lam = proj_torad(lon_deg);
    in_geo.lpzt.phi = proj_torad(lat_deg);
    in_geo.lpzt.z   = h;
    in_geo.lpzt.t   = t_epoch;

    PJ_COORD cart = proj_trans(P_cart, PJ_FWD, in_geo);
    proj_destroy(P_cart);

    // 2) Apply the deformation grid in Cartesian space.
    std::ostringstream ss2;
    ss2 << "+proj=deformation +t_epoch=" << ref_epoch
        << " +grids=" << grid_path;

    PJ* P_def = proj_create(ctx_, ss2.str().c_str());
    if (!P_def) {
        throw std::runtime_error("Failed to create PROJ deformation transformer: " + grid_path);
    }

    PJ_COORD in_cart2;
    in_cart2.xyzt.x = cart.xyzt.x;
    in_cart2.xyzt.y = cart.xyzt.y;
    in_cart2.xyzt.z = cart.xyzt.z;
    in_cart2.xyzt.t = t_epoch;

    PJ_COORD cart2 = proj_trans(P_def, PJ_FWD, in_cart2);
    proj_destroy(P_def);

    // 3) Convert Cartesian → geodetic.
    PJ* P_cart_inv = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart_inv) {
        throw std::runtime_error("Failed to create PROJ inverse cart converter");
    }

    PJ_COORD out_geo = proj_trans(P_cart_inv, PJ_INV, cart2);
    proj_destroy(P_cart_inv);

    // Return transformed coordinates in degrees.
    return { proj_todeg(out_geo.lpzt.lam),
             proj_todeg(out_geo.lpzt.phi),
             out_geo.lpzt.z,
             out_geo.lpzt.t };
}