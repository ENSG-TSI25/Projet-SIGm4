#include <core/TransformationEngine.hpp>
#include <vector>
#include <iostream>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_spatialref.h>

// ----------------------------------------------------
// TRANSFORM LAYER AT EPOCH BETWEEN CRS
// ----------------------------------------------------

GeodeticTransformer::Result TransformationEngine::transformPoint(OGRPoint * p, double t, const std::string &fmt_in, const std::string &fmt_out, const std::string &src_code, const std::string &dst_code)
{
    Result r;

    if (fmt_in == "geocentric")
    {
        if (fmt_out == "geocentric")
            r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
        else if (fmt_out == "geodetic")
            r = GeodeticTransformer::geocentricToGeodetic(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
        else
            r = GeodeticTransformer::geocentricToProjected(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
    }
    else if (fmt_in == "geodetic")
    {
        if (fmt_out == "geocentric")
            r = GeodeticTransformer::geodeticToGeocentric(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
        else if (fmt_out == "geodetic")
            r = GeodeticTransformer::transformGeodeticAtEpoch(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
        else
            r = GeodeticTransformer::geodeticToProjected(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
    }
    else // projected
    {
        if (fmt_out == "geocentric")
            r = GeodeticTransformer::projectedToGeocentric(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
        else if (fmt_out == "geodetic")
            r = GeodeticTransformer::projectedToGeodetic(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
        else
            r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), t, src_code, dst_code);
    }

    return r;
}

VectorLayer *TransformationEngine::transformLayerAtEpoch(VectorLayer &inputLayer, const std::string &epsg_dst)
{
    auto geometries = inputLayer.getGeometries();

    // 1. Fetch source and destination EPSG codes
    std::string src_code = inputLayer.getCrs();
    if (src_code.find("EPSG:") == std::string::npos)
        src_code = "EPSG:" + src_code;
    int src_code_int = std::stoi(src_code.substr(src_code.find(":") + 1));

    std::string dst_code = epsg_dst;
    if (dst_code.find("EPSG:") == std::string::npos)
        dst_code = "EPSG:" + dst_code;
    int dst_code_int = std::stoi(dst_code.substr(dst_code.find(":") + 1));

    // 2. Determinate format types
    std::string fmt_in = GeodeticTransformer::getCRSFormat(src_code_int);
    if (fmt_in == "unknown")
    {
        OGRSpatialReference srs;
        srs.SetFromUserInput(src_code.c_str());
        if (srs.IsGeocentric())
            fmt_in = "geocentric";
        else if (srs.IsGeographic())
            fmt_in = "geodetic";
        else
            fmt_in = "projected";
    }

    std::string fmt_out = GeodeticTransformer::getCRSFormat(dst_code_int);
    if (fmt_out == "unknown")
    {
        OGRSpatialReference srs;
        srs.SetFromUserInput(dst_code.c_str());
        if (srs.IsGeocentric())
            fmt_out = "geocentric";
        else if (srs.IsGeographic())
            fmt_out = "geodetic";
        else
            fmt_out = "projected";
    }

    std::cout << "TRANSFORMATION : " << src_code << " (" << fmt_in << ") -> "
              << dst_code << " (" << fmt_out << ")\n";

    // 3. Exécution
    for (auto &g : geometries)
    {
        OGRGeometry *geom = g->getGeometry();
        if (!geom)
            continue;
        else
        {
            switch (wkbFlatten(geom->getGeometryType()))
            {
            case wkbPoint:{
                OGRPoint *p = geom->toPoint();

                Result r;
                r = transformPoint(p, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                g->setT(r.t);
                p->setX(r.x);
                p->setY(r.y);
                p->setZ(r.z);

                break;
            }

            case wkbLineString:{
                OGRLineString *l = geom->toLineString();
                int num_l = l->getNumPoints();
                for (int i = 0; i < num_l; i++)
                {
                    Result r;
                    OGRPoint pt; l->getPoint(i, &pt);
                    r = transformPoint(&pt, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                    g->setT(r.t);
                    l->setPoint(i, r.x, r.y, r.z);
                }
                    break;
            }
                case wkbPolygon:{
                    OGRPolygon *poly = geom->toPolygon();

                    // Exterior ring
                    OGRLinearRing *ext = poly->getExteriorRing();

                    for (int i = 0; i < ext->getNumPoints(); i++)
                    {
                        Result r;
                        OGRPoint pt; ext->getPoint(i, &pt);
                        r = transformPoint(&pt, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                        g->setT(r.t);
                        ext->setPoint(i, r.x, r.y, r.z);
                    }

                    // Interior rings
                    for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++)
                    {
                        OGRLinearRing *inter = poly->getInteriorRing(r_id);
                        for (int i = 0; i < inter->getNumPoints(); i++)
                        {
                            Result r;
                            OGRPoint pt; inter->getPoint(i, &pt);
                            r = transformPoint(&pt, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                            g->setT(r.t);
                            inter->setPoint(i, r.x, r.y, r.z);
                        }
                    }

                    break;
            }
                case wkbMultiPoint:
                {
                    OGRMultiPoint *mpt = geom->toMultiPoint();
                    int num_mpt = mpt->getNumGeometries();

                    for (int i = 0; i < num_mpt; i++)
                    {
                        OGRPoint *p = (OGRPoint *)mpt->getGeometryRef(i);
                        Result r;

                        r = transformPoint(p, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                        g->setT(r.t);
                        p->setX(r.x);
                        p->setY(r.y);
                        p->setZ(r.z);
                    }

                    break;
                }
                case wkbMultiLineString:{
                        OGRMultiLineString *ml = geom->toMultiLineString();
                        int num_ml = ml->getNumGeometries();

                        for (int i = 0; i < num_ml; i++)
                        {
                            OGRLineString *l = (OGRLineString *)ml->getGeometryRef(i);
                            int n_pts = l->getNumPoints();

                            for (int j = 0; j < n_pts; j++)
                            {
                                Result r;
                                OGRPoint pt; l->getPoint(j, &pt);
                                r = transformPoint(&pt, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                                g->setT(r.t);
                                l->setPoint(j, r.x, r.y, r.z); 
                            }
                        }
                        break;  
                    }
                    
                    case wkbMultiPolygon:{
                        OGRMultiPolygon *mpl = geom->toMultiPolygon();
                        int num_mpl = mpl->getNumGeometries();

                        for (int i = 0; i < num_mpl; i++)
                        {
                            OGRPolygon *poly = (OGRPolygon *)mpl->getGeometryRef(i);
                            OGRLinearRing *ext = poly->getExteriorRing();

                            // Exterior ring
                            for (int j = 0; j < ext->getNumPoints(); j++)
                            {
                                Result r;
                                OGRPoint pt; ext->getPoint(j, &pt);
                                r = transformPoint(&pt, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                                g->setT(r.t);
                                ext->setPoint(j, r.x, r.y, r.z);
                            }
                            
                            // Interior rings
                            for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++)
                            {
                                OGRLinearRing *inter = poly->getInteriorRing(r_id);
                                for (int k = 0; k < inter->getNumPoints(); k++)
                                {
                                    Result r;
                                    OGRPoint pt; inter->getPoint(k, &pt);
                                    r = transformPoint(&pt, g->getT(), fmt_in, fmt_out, src_code, dst_code);

                                    g->setT(r.t);
                                    inter->setPoint(k, r.x, r.y, r.z);
                                }
                            }
                        }
                        break; 
                    }
                    
                    default:
                        break;
            } 
        }
    }  

    // Mise à jour finale du Layer
    inputLayer.setCrs(dst_code);
    return &inputLayer;
}  


// ----------------------------------------------------
// 2. JSON DEF-MODEL DEFORMATION (IGN, Mayotte, etc.)
// ----------------------------------------------------


GeodeticTransformer::Result TransformationEngine::transformPointDefModel(
    OGRPoint *p, double t, 
    const std::string &fmt_in, 
    const std::string &model_path, int src_epsg, bool inverse)
{
    using GT = GeodeticTransformer;
    if (fmt_in == "geocentric") {
        return GT::applyDefModelGeocentric(p->getX(), p->getY(), p->getZ(), t, model_path, inverse);
    } else if (fmt_in == "geodetic") {
        return GT::applyDefModelGeodetic(p->getX(), p->getY(), p->getZ(), t, model_path, inverse);
    } else { // projected
        return GT::applyDefModelProjected(p->getX(), p->getY(), p->getZ(), t, model_path, src_epsg, inverse);
    }
}


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
        if (!geom) continue;

        switch (wkbFlatten(geom->getGeometryType()))
        {
            case wkbPoint: {
                OGRPoint* p = geom->toPoint();
                Result r;
                r = transformPointDefModel(p, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                g->setT(r.t);
                p->setX(r.x); p->setY(r.y); p->setZ(r.z);
                break;
            }

            case wkbLineString: {
                OGRLineString* l = geom->toLineString();
                int num_l = l->getNumPoints();
                for (int i = 0; i < num_l; i++) {
                    Result r;
                    OGRPoint pt; l->getPoint(i, &pt);
                    r = transformPointDefModel(&pt, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                    l->setPoint(i, r.x, r.y, r.z);
                    if (i == 0) g->setT(r.t);
                }
                break;
            }

            case wkbPolygon: {
                OGRPolygon* poly = geom->toPolygon();
                
                // Exterior ring
                OGRLinearRing* ext = poly->getExteriorRing();
                if (ext) {
                    for (int i = 0; i < ext->getNumPoints(); i++) {
                        Result r;
                        OGRPoint pt; ext->getPoint(i, &pt);
                        r = transformPointDefModel(&pt, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                        ext->setPoint(i, r.x, r.y, r.z);
                        if (i == 0) g->setT(r.t);
                    }
                }

                // Interior rings
                for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++) {
                    OGRLinearRing* inter = poly->getInteriorRing(r_id);
                    if (inter) {
                        for (int i = 0; i < inter->getNumPoints(); i++) {
                            Result r;
                            OGRPoint pt; inter->getPoint(i, &pt);
                            r = transformPointDefModel(&pt, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                            inter->setPoint(i, r.x, r.y, r.z);
                        }
                    }
                }
                break;
            }

            case wkbMultiPoint: {
                OGRMultiPoint* mpt = geom->toMultiPoint();
                int num_mpt = mpt->getNumGeometries();
                for (int i = 0; i < num_mpt; i++) {
                    OGRPoint* p = (OGRPoint*)mpt->getGeometryRef(i);
                    Result r;
                    r = transformPointDefModel(p, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                    p->setX(r.x); p->setY(r.y); p->setZ(r.z);
                    if (i == 0) g->setT(r.t);
                }
                break;
            }

            case wkbMultiLineString: {
                OGRMultiLineString* ml = geom->toMultiLineString();
                int num_ml = ml->getNumGeometries();
                for (int i = 0; i < num_ml; i++) {
                    OGRLineString* l = (OGRLineString*)ml->getGeometryRef(i);
                    int n_pts = l->getNumPoints();
                    for (int j = 0; j < n_pts; j++) {
                        Result r;
                        OGRPoint pt; l->getPoint(j, &pt);
                        r = transformPointDefModel(&pt, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                        l->setPoint(j, r.x, r.y, r.z);
                        if (i == 0 && j == 0) g->setT(r.t);
                    }
                }
                break;
            }

            case wkbMultiPolygon: {
                OGRMultiPolygon* mpl = geom->toMultiPolygon();
                int num_mpl = mpl->getNumGeometries();
                for (int i = 0; i < num_mpl; i++) {
                    OGRPolygon* poly = (OGRPolygon*)mpl->getGeometryRef(i);

                    // Exterior ring
                    OGRLinearRing* ext = poly->getExteriorRing();
                    if (ext) {
                        for (int j = 0; j < ext->getNumPoints(); j++) {
                            Result r;
                            OGRPoint pt; ext->getPoint(j, &pt);
                            r = transformPointDefModel(&pt, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                            ext->setPoint(j, r.x, r.y, r.z);
                            if (i == 0 && j == 0) g->setT(r.t);
                        }
                    }
                    // Interior rings
                    for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++) {
                        OGRLinearRing* inter = poly->getInteriorRing(r_id);
                        if (inter) {
                            for (int k = 0; k < inter->getNumPoints(); k++) {
                                Result r;
                                OGRPoint pt; inter->getPoint(k, &pt);
                                r = transformPointDefModel(&pt, g->getT(), fmt_in, json_model_path, src_code_int, inverse);

                                inter->setPoint(k, r.x, r.y, r.z);
                            }
                        }
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    return &inputLayer;
}

// // -------------------------------
// // 3. GRID-BASED DEFORMATION MODEL
// // -------------------------------

GeodeticTransformer::Result TransformationEngine::transformPointGrid(
    OGRPoint *p, double t, 
    const std::string &fmt_in, 
    const std::string &grid_path, int src_epsg, double ref_epoch)
{
    using GT = GeodeticTransformer;
    if (fmt_in == "geocentric") {
        return GT::applyGridDeformationGeocentric(p->getX(), p->getY(), p->getZ(), t, grid_path, ref_epoch);
    } else if (fmt_in == "geodetic") {
        return GT::applyGridDeformationGeodetic(p->getX(), p->getY(), p->getZ(), t, grid_path, ref_epoch);
    } else { // projected
        return GT::applyGridDeformationProjected(p->getX(), p->getY(), p->getZ(), t, grid_path, src_epsg, ref_epoch);
    }
}
VectorLayer* TransformationEngine::applyGridDeformationLayer(VectorLayer& inputLayer, const std::string& grid_path, double ref_epoch)
{
    auto geometries = inputLayer.getGeometries();
    
    // 1. Fetch source EPSG code
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
        if (!geom) continue;

        switch (wkbFlatten(geom->getGeometryType()))
        {
            case wkbPoint: {
                OGRPoint* p = geom->toPoint();
                Result r;
                r = transformPointGrid(p, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                g->setT(r.t);
                p->setX(r.x); p->setY(r.y); p->setZ(r.z);
                break;
            }

            case wkbLineString: {
                OGRLineString* l = geom->toLineString();
                int num_l = l->getNumPoints();
                for (int i = 0; i < num_l; i++) {
                    Result r;
                    OGRPoint pt; l->getPoint(i, &pt);
                    r = transformPointGrid(&pt, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                    l->setPoint(i, r.x, r.y, r.z);
                    if (i == 0) g->setT(r.t);
                }
                break;
            }

            case wkbPolygon: {
                OGRPolygon* poly = geom->toPolygon();
                
                // Exterior ring
                OGRLinearRing* ext = poly->getExteriorRing();
                if (ext) {
                    for (int i = 0; i < ext->getNumPoints(); i++) {
                        Result r;
                        OGRPoint pt; ext->getPoint(i, &pt);
                        r = transformPointGrid(&pt, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                        ext->setPoint(i, r.x, r.y, r.z);
                        if (i == 0) g->setT(r.t);
                    }
                }

                // Interior rings
                for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++) {
                    OGRLinearRing* inter = poly->getInteriorRing(r_id);
                    if (inter) {
                        for (int i = 0; i < inter->getNumPoints(); i++) {
                            Result r;
                            OGRPoint pt; inter->getPoint(i, &pt);
                            r = transformPointGrid(&pt, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                            inter->setPoint(i, r.x, r.y, r.z);
                        }
                    }
                }
                break;
            }

            case wkbMultiPoint: {
                OGRMultiPoint* mpt = geom->toMultiPoint();
                int num_mpt = mpt->getNumGeometries();
                for (int i = 0; i < num_mpt; i++) {
                    OGRPoint* p = (OGRPoint*)mpt->getGeometryRef(i);
                    Result r;
                    r = transformPointGrid(p, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                    p->setX(r.x); p->setY(r.y); p->setZ(r.z);
                    if (i == 0) g->setT(r.t);
                }
                break;
            }

            case wkbMultiLineString: {
                OGRMultiLineString* ml = geom->toMultiLineString();
                int num_ml = ml->getNumGeometries();
                for (int i = 0; i < num_ml; i++) {
                    OGRLineString* l = (OGRLineString*)ml->getGeometryRef(i);
                    int n_pts = l->getNumPoints();
                    for (int j = 0; j < n_pts; j++) {
                        Result r;
                        OGRPoint pt; l->getPoint(j, &pt);
                        r = transformPointGrid(&pt, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                        l->setPoint(j, r.x, r.y, r.z);
                        if (i == 0 && j == 0) g->setT(r.t);
                    }
                }
                break;
            }

            case wkbMultiPolygon: {
                OGRMultiPolygon* mpl = geom->toMultiPolygon();
                int num_mpl = mpl->getNumGeometries();
                for (int i = 0; i < num_mpl; i++) {
                    OGRPolygon* poly = (OGRPolygon*)mpl->getGeometryRef(i);
                    
                    // Exterior ring
                    OGRLinearRing* ext = poly->getExteriorRing();
                    if (ext) {
                        for (int j = 0; j < ext->getNumPoints(); j++) {
                            Result r;
                            OGRPoint pt; ext->getPoint(j, &pt);
                            r = transformPointGrid(&pt, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                            ext->setPoint(j, r.x, r.y, r.z);
                            if (i == 0 && j == 0) g->setT(r.t);
                        }
                    }
                    // Interior rings
                    for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++) {
                        OGRLinearRing* inter = poly->getInteriorRing(r_id);
                        if (inter) {
                            for (int k = 0; k < inter->getNumPoints(); k++) {
                                Result r;
                                OGRPoint pt; inter->getPoint(k, &pt);
                                r = transformPointGrid(&pt, g->getT(), fmt_in, grid_path, src_code_int, ref_epoch);

                                inter->setPoint(k, r.x, r.y, r.z);
                            }
                        }
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    return &inputLayer;
}