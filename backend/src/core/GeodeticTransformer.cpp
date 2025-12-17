#include "core/GeodeticTransformer.hpp"

#include <proj/io.hpp>
#include <proj/crs.hpp>
#include <proj/coordinateoperation.hpp>
#include <proj/util.hpp>

#include <stdexcept>
#include <memory>
#include <sstream>
#include <iostream>
#include <cmath>

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

    // OUTPUT : PROJ returns Lat(x), Lon(y) so they are flipped.
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
    in.xyzt.x = y;
    in.xyzt.y = x;
    in.xyzt.z = z; 
    in.xyzt.t = t_epoch;

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
    std::ostringstream proj_str;
    proj_str << "+proj=pipeline "
       << "+step +proj=cart +inv +ellps=GRS80 " 
       << "+step +proj=defmodel +model=" << json_model_path; 

    if (inverse) {
        proj_str << " +inv";
    }

    proj_str << " +step +proj=cart +ellps=GRS80";

    PJ* P = proj_create(ctx_, proj_str.str().c_str());
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
    int epsg_projected,
    bool inverse)
{
    std::string epsg_str = "EPSG:" + std::to_string(epsg_projected);

    // 1) Projected -> Geodetic
    Result geo = projectedToGeodetic(E, N, H, t_epoch,
                                     epsg_str,
                                     "EPSG:4979"); 

    // 2) Appliquer le modèle déf
    Result def = applyDefModelGeodetic(geo.x, geo.y, geo.z, t_epoch, json_model_path, inverse);

    // 3) Geodetic -> Projected
    Result proj = geodeticToProjected(def.x, def.y, def.z, def.t,
                                      "EPSG:4979",
                                      epsg_str);

    return proj;
}

// ----------------------------------------------------
// 12. applyGridDeformation - GEODETIC (lon, lat, h)
// ----------------------------------------------------

GeodeticTransformer::Result GeodeticTransformer::applyGridDeformationGeodetic(
    double lon_deg, double lat_deg, double h, double t_epoch,
    const std::string& grid_path,
    double ref_epoch)
{
    // 1) Geodetic -> Cartesian (XYZ)
    PJ* P_cart = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart)
        throw std::runtime_error("Failed to create PROJ cart converter");

    PJ_COORD in_geo;
    in_geo.lpzt.lam = proj_torad(lon_deg);
    in_geo.lpzt.phi = proj_torad(lat_deg);
    in_geo.lpzt.z   = h;
    in_geo.lpzt.t   = t_epoch;

    PJ_COORD cart = proj_trans(P_cart, PJ_FWD, in_geo);
    proj_destroy(P_cart);

    // 2) Apply deformation grid (XYZ)
    std::ostringstream ss;
    ss << "+proj=deformation +t_epoch=" << ref_epoch
       << " +grids=" << grid_path;

    PJ* P_def = proj_create(ctx_, ss.str().c_str());
    if (!P_def)
        throw std::runtime_error("Failed to create PROJ deformation transformer");

    PJ_COORD cart2 = proj_trans(P_def, PJ_FWD, cart);
    proj_destroy(P_def);

    // 3) Cartesian -> Geodetic
    PJ* P_cart_inv = proj_create(ctx_, "+proj=cart +ellps=GRS80");
    if (!P_cart_inv)
        throw std::runtime_error("Failed to create PROJ cart inverse converter");

    PJ_COORD out_geo = proj_trans(P_cart_inv, PJ_INV, cart2);
    proj_destroy(P_cart_inv);

    return {
        proj_todeg(out_geo.lpzt.lam),
        proj_todeg(out_geo.lpzt.phi),
        out_geo.lpzt.z,
        out_geo.lpzt.t
    };
}

// ----------------------------------------------------
// 13. applyGridDeformation - GEOCENTRIC (X, Y, Z)
// ----------------------------------------------------
GeodeticTransformer::Result GeodeticTransformer::applyGridDeformationGeocentric(
    double X, double Y, double Z, double t_epoch,
    const std::string& grid_path,
    double ref_epoch)
{
    std::ostringstream ss;
    ss << "+proj=deformation +t_epoch=" << ref_epoch
       << " +grids=" << grid_path
       << " +ellps=GRS80";

    PJ* P = proj_create(ctx_, ss.str().c_str());
    if (!P)
        throw std::runtime_error("Failed to create PROJ deformation transformer");

    PJ_COORD in;
    in.xyzt.x = X;
    in.xyzt.y = Y;
    in.xyzt.z = Z;
    in.xyzt.t = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, in);
    proj_destroy(P);

    return {
        r.xyzt.x,
        r.xyzt.y,
        r.xyzt.z,
        r.xyzt.t
    };
}

// ----------------------------------------------------
// 14. applyGridDeformation - PROJECTED (E, N, H)
// ----------------------------------------------------
GeodeticTransformer::Result GeodeticTransformer::applyGridDeformationProjected(
    double E, double N, double H, double t_epoch,
    const std::string& grid_path,
    int epsg_projected,
    double ref_epoch)
{
    std::string epsg_str = "EPSG:" + std::to_string(epsg_projected);

    // 1) Projected -> Geocentric
    Result cart = projectedToGeocentric(E, N, H, t_epoch, epsg_str, "EPSG:4978");

    // 3) Appliquer la grille de déformation
    Result cart_def = applyGridDeformationGeocentric(cart.x, cart.y, cart.z, t_epoch, grid_path, ref_epoch);

    // 4) Geocentric -> Projected
    Result proj = geocentricToProjected(cart_def.x, cart_def.y, cart_def.z, cart_def.t, "EPSG:4978", epsg_str);

    return proj;
}
