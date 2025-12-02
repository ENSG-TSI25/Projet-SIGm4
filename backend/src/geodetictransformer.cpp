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

GeodeticTransformer::GeodeticTransformer() {
    ctx_ = proj_context_create();
}

GeodeticTransformer::~GeodeticTransformer() {
    proj_context_destroy(ctx_);
}

//
// 1. TRANSFORMATION ITRF/ETRF/etc. À ÉPOQUE
//
GeodeticTransformer::Result
GeodeticTransformer::transformAtEpoch(
    double x, double y, double z, double t_epoch,
    const std::string& epsg_src,
    const std::string& epsg_dst)
{
    auto db = DatabaseContext::create();
    auto af = AuthorityFactory::create(db, "EPSG");

    auto source = af->createCoordinateReferenceSystem(epsg_src);
    auto target = af->createCoordinateReferenceSystem(epsg_dst);

    auto opCtx = CoordinateOperationContext::create(
        AuthorityFactory::create(db, std::string()),  // authorityFactory
        nullptr,                                      // sourceCRS (pas utilisé)
        0.0                                           // desired accuracy
        );

    auto ops = CoordinateOperationFactory::create()->createOperations(
        source, target, opCtx);

    if (ops.empty())
        throw std::runtime_error("No coordinate operation found.");

    auto tr = ops[0]->coordinateTransformer(ctx_);

    PJ_COORD c;
    c.xyzt.x = x;
    c.xyzt.y = y;
    c.xyzt.z = z;
    c.xyzt.t = t_epoch;

    PJ_COORD r = tr->transform(c);

    return { r.xyzt.x, r.xyzt.y, r.xyzt.z, r.xyzt.t };
}

//
// 2. DÉFORMATION JSON (Mayotte etc.)
//
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

    PJ_COORD c;
    c.xyzt.x = lon;
    c.xyzt.y = lat;
    c.xyzt.z = h;
    c.xyzt.t = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, c);
    proj_destroy(P);

    return { r.xyzt.x, r.xyzt.y, r.xyzt.z, r.xyzt.t };
}

//
// 3. DÉFORMATION VIA GRILLE (Scandinavie etc.)
//
GeodeticTransformer::Result
GeodeticTransformer::applyGridDeformation(
    double x, double y, double z, double t_epoch,
    const std::string& grid_path,
    double ref_epoch)
{
    std::ostringstream ss;
    ss << "+proj=deformation +t_epoch=" << ref_epoch
       << " +grids=" << grid_path;

    PJ* P = proj_create(ctx_, ss.str().c_str());
    if (!P)
        throw std::runtime_error("Failed to load deformation grid: " + grid_path);

    PJ_COORD c;
    c.xyzt.x = x;
    c.xyzt.y = y;
    c.xyzt.z = z;
    c.xyzt.t = t_epoch;

    PJ_COORD r = proj_trans(P, PJ_FWD, c);
    proj_destroy(P);

    return { r.xyzt.x, r.xyzt.y, r.xyzt.z, r.xyzt.t };
}
