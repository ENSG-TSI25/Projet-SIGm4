#include <core/TransformationEngine.hpp>
#include <vector>
#include <iostream>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_spatialref.h>

// // --------------------------------------------------------------
// // 1. TIME-DEPENDENT COORDINATE TRANSFORMATION (ITRF, ETRF, etc.)
// // --------------------------------------------------------------

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
            switch (geom->getGeometryType())
            {
            case wkbPoint:{
                OGRPoint *p = geom->toPoint(); // Use envelope

                Result r;

                // --- MATRICE DE DECISION (3 x 3 cas) ---

                // CAS 1 : Geocentric (XYZ) en entrée
                if (fmt_in == "geocentric")
                {
                    if (fmt_out == "geocentric")
                    {
                        r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                    else if (fmt_out == "geodetic")
                    {
                        r = GeodeticTransformer::geocentricToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                    else
                    { // projected
                        r = GeodeticTransformer::geocentricToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                }
                // CAS 2 : Geodetic (Lat/Lon) en entrée
                else if (fmt_in == "geodetic")
                {
                    if (fmt_out == "geocentric")
                    {
                        r = GeodeticTransformer::geodeticToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                    else if (fmt_out == "geodetic")
                    {
                        r = GeodeticTransformer::transformGeodeticAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                    else
                    { // projected
                        r = GeodeticTransformer::geodeticToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                }
                // CAS 3 : Projected (E/N) en entrée
                else
                { // projected
                    if (fmt_out == "geocentric")
                    {
                        r = GeodeticTransformer::projectedToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                    else if (fmt_out == "geodetic")
                    {
                        r = GeodeticTransformer::projectedToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                    else
                    { // projected
                        r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                    }
                }

                // Mise à jour du point
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
                    double x = l->getX(i);
                    double y = l->getY(i);
                    double z = l->getZ(i);
                    
                    Result r;

                    if (fmt_in == "geocentric")
                    {
                        if (fmt_out == "geocentric")
                        {
                            r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else if (fmt_out == "geodetic")
                        {
                            r = GeodeticTransformer::geocentricToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else
                        { // projected
                            r = GeodeticTransformer::geocentricToProjected(x, y, z, g->getT(), src_code, dst_code);
                        }
                    }
                    // CAS 2 : Geodetic (Lat/Lon) en entrée
                    else if (fmt_in == "geodetic")
                    {
                        if (fmt_out == "geocentric")
                        {
                            r = GeodeticTransformer::geodeticToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else if (fmt_out == "geodetic")
                        {
                            r = GeodeticTransformer::transformGeodeticAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else
                        { // projected
                            r = GeodeticTransformer::geodeticToProjected(x, y, z, g->getT(), src_code, dst_code);
                        }
                    }
                    // CAS 3 : Projected (E/N) en entrée
                    else
                    { // projected
                        if (fmt_out == "geocentric")
                        {
                            r = GeodeticTransformer::projectedToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else if (fmt_out == "geodetic")
                        {
                            r = GeodeticTransformer::projectedToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else
                        { // projected
                            r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                        }
                    }

                    // Mise à jour du point
                    g->setT(r.t);
                    l->setPoint(i, r.x, r.y, r.z);
                }
                    break;
            }
                case wkbPolygon:{
                    OGRPolygon *poly = geom->toPolygon();

                    // Parcourir l'anneau extérieur
                    OGRLinearRing *ext = poly->getExteriorRing();

                    for (int i = 0; i < ext->getNumPoints(); i++)
                    {
                        double x = ext->getX(i);
                        double y = ext->getY(i);
                        double z = ext->getZ(i);

                        Result r;

                        // --- MATRICE DE DECISION (3 x 3 cas) ---
                        if (fmt_in == "geocentric")
                        {
                            if (fmt_out == "geocentric")
                                r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                            else if (fmt_out == "geodetic")
                                r = GeodeticTransformer::geocentricToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                            else
                                r = GeodeticTransformer::geocentricToProjected(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else if (fmt_in == "geodetic")
                        {
                            if (fmt_out == "geocentric")
                                r = GeodeticTransformer::geodeticToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                            else if (fmt_out == "geodetic")
                                r = GeodeticTransformer::transformGeodeticAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                            else
                                r = GeodeticTransformer::geodeticToProjected(x, y, z, g->getT(), src_code, dst_code);
                        }
                        else
                        {
                            if (fmt_out == "geocentric")
                                r = GeodeticTransformer::projectedToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                            else if (fmt_out == "geodetic")
                                r = GeodeticTransformer::projectedToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                            else
                                r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                        }
                        g->setT(r.t);
                        ext->setPoint(i, r.x, r.y, r.z);
                    }

                    // Parcourir les anneaux intérieurs (trous)
                    for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++)
                    {
                        OGRLinearRing *inter = poly->getInteriorRing(r_id);
                        for (int i = 0; i < inter->getNumPoints(); i++)
                        {
                            double x = inter->getX(i);
                            double y = inter->getY(i);
                            double z = inter->getZ(i);

                            Result r;

                            // --- Même matrice de décision ---
                            if (fmt_in == "geocentric")
                            {
                                if (fmt_out == "geocentric")
                                    r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                else if (fmt_out == "geodetic")
                                    r = GeodeticTransformer::geocentricToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                else
                                    r = GeodeticTransformer::geocentricToProjected(x, y, z, g->getT(), src_code, dst_code);
                            }
                            else if (fmt_in == "geodetic")
                            {
                                if (fmt_out == "geocentric")
                                    r = GeodeticTransformer::geodeticToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                else if (fmt_out == "geodetic")
                                    r = GeodeticTransformer::transformGeodeticAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                else
                                    r = GeodeticTransformer::geodeticToProjected(x, y, z, g->getT(), src_code, dst_code);
                            }
                            else
                            {
                                if (fmt_out == "geocentric")
                                    r = GeodeticTransformer::projectedToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                else if (fmt_out == "geodetic")
                                    r = GeodeticTransformer::projectedToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                else
                                    r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                            }

                            // Mise à jour du point
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

                        // --- MATRICE DE DECISION (3 x 3 cas) ---
                        if (fmt_in == "geocentric")
                        {
                            if (fmt_out == "geocentric")
                                r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                            else if (fmt_out == "geodetic")
                                r = GeodeticTransformer::geocentricToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                            else
                                r = GeodeticTransformer::geocentricToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                        }
                        else if (fmt_in == "geodetic")
                        {
                            if (fmt_out == "geocentric")
                                r = GeodeticTransformer::geodeticToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                            else if (fmt_out == "geodetic")
                                r = GeodeticTransformer::transformGeodeticAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                            else
                                r = GeodeticTransformer::geodeticToProjected(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                        }
                        else // projected
                        {
                            if (fmt_out == "geocentric")
                                r = GeodeticTransformer::projectedToGeocentric(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                            else if (fmt_out == "geodetic")
                                r = GeodeticTransformer::projectedToGeodetic(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                            else
                                r = GeodeticTransformer::transformLinearAtEpoch(p->getX(), p->getY(), p->getZ(), g->getT(), src_code, dst_code);
                        }

                        // Mise à jour du point
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
                                double x = l->getX(j);
                                double y = l->getY(j);
                                double z = l->getZ(j);

                                Result r;
                                // --- MATRICE DE DECISION (3 x 3 cas) ---
                                if (fmt_in == "geocentric")
                                {
                                    if (fmt_out == "geocentric")
                                        r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                    else if (fmt_out == "geodetic")
                                        r = GeodeticTransformer::geocentricToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                    else
                                        r = GeodeticTransformer::geocentricToProjected(x, y, z, g->getT(), src_code, dst_code);
                                }
                                else if (fmt_in == "geodetic")
                                {
                                    if (fmt_out == "geocentric")
                                        r = GeodeticTransformer::geodeticToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                    else if (fmt_out == "geodetic")
                                        r = GeodeticTransformer::transformGeodeticAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                    else
                                        r = GeodeticTransformer::geodeticToProjected(x, y, z, g->getT(), src_code, dst_code);
                                }
                                else // projected
                                {
                                    if (fmt_out == "geocentric")
                                        r = GeodeticTransformer::projectedToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                    else if (fmt_out == "geodetic")
                                        r = GeodeticTransformer::projectedToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                    else
                                        r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                }

                                // Mise à jour du point
                                g->setT(r.t);
                                l->setPoint(j, r.x, r.y, r.z);  // Correction: j au lieu de i
                            }
                        }
                        break;  // Break à la fin du case
                    }
                    
                    case wkbMultiPolygon:{
                        OGRMultiPolygon *mpl = geom->toMultiPolygon();
                        int num_mpl = mpl->getNumGeometries();

                        for (int i = 0; i < num_mpl; i++)
                        {
                            OGRPolygon *poly = (OGRPolygon *)mpl->getGeometryRef(i);
                            OGRLinearRing *ext = poly->getExteriorRing();

                            for (int j = 0; j < ext->getNumPoints(); j++)
                            {
                                double x = ext->getX(j);
                                double y = ext->getY(j);
                                double z = ext->getZ(j);

                                Result r;

                                // --- MATRICE DE DECISION (3 x 3 cas) ---
                                if (fmt_in == "geocentric")
                                {
                                    if (fmt_out == "geocentric")
                                        r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                    else if (fmt_out == "geodetic")
                                        r = GeodeticTransformer::geocentricToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                    else
                                        r = GeodeticTransformer::geocentricToProjected(x, y, z, g->getT(), src_code, dst_code);
                                }
                                else if (fmt_in == "geodetic")
                                {
                                    if (fmt_out == "geocentric")
                                        r = GeodeticTransformer::geodeticToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                    else if (fmt_out == "geodetic")
                                        r = GeodeticTransformer::transformGeodeticAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                    else
                                        r = GeodeticTransformer::geodeticToProjected(x, y, z, g->getT(), src_code, dst_code);
                                }
                                else
                                {
                                    if (fmt_out == "geocentric")
                                        r = GeodeticTransformer::projectedToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                    else if (fmt_out == "geodetic")
                                        r = GeodeticTransformer::projectedToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                    else
                                        r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                }
                                g->setT(r.t);
                                ext->setPoint(j, r.x, r.y, r.z);
                            }
                            
                            // Parcourir les anneaux intérieurs (trous)
                            for (int r_id = 0; r_id < poly->getNumInteriorRings(); r_id++)
                            {
                                OGRLinearRing *inter = poly->getInteriorRing(r_id);
                                for (int k = 0; k < inter->getNumPoints(); k++)
                                {
                                    double x = inter->getX(k);
                                    double y = inter->getY(k);
                                    double z = inter->getZ(k);

                                    Result r;

                                    // --- Même matrice de décision ---
                                    if (fmt_in == "geocentric")
                                    {
                                        if (fmt_out == "geocentric")
                                            r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                        else if (fmt_out == "geodetic")
                                            r = GeodeticTransformer::geocentricToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                        else
                                            r = GeodeticTransformer::geocentricToProjected(x, y, z, g->getT(), src_code, dst_code);
                                    }
                                    else if (fmt_in == "geodetic")
                                    {
                                        if (fmt_out == "geocentric")
                                            r = GeodeticTransformer::geodeticToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                        else if (fmt_out == "geodetic")
                                            r = GeodeticTransformer::transformGeodeticAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                        else
                                            r = GeodeticTransformer::geodeticToProjected(x, y, z, g->getT(), src_code, dst_code);
                                    }
                                    else
                                    {
                                        if (fmt_out == "geocentric")
                                            r = GeodeticTransformer::projectedToGeocentric(x, y, z, g->getT(), src_code, dst_code);
                                        else if (fmt_out == "geodetic")
                                            r = GeodeticTransformer::projectedToGeodetic(x, y, z, g->getT(), src_code, dst_code);
                                        else
                                            r = GeodeticTransformer::transformLinearAtEpoch(x, y, z, g->getT(), src_code, dst_code);
                                    }

                                    // Mise à jour du point
                                    g->setT(r.t);
                                    inter->setPoint(k, r.x, r.y, r.z);
                                }
                            }
                        }
                        break;  // Break manquant
                    }
                    
                    default:
                        break;
            }  // Fin du switch
        }
    }  // Fin de la boucle for (auto &g : geometries)

    // Mise à jour finale du Layer
    inputLayer.setCrs(dst_code);
    return &inputLayer;
}  // Fin de la fonction


GeodeticTransformer::Result TransformationEngine::transformPoint(OGRPoint * p, double &t, const std::string &fmt_in, const std::string &fmt_out, const std::string &src_code, const std::string &dst_code)
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

// // ----------------------------------------------------
// // 2. JSON DEF-MODEL DEFORMATION (IGN, Mayotte, etc.)
// // ----------------------------------------------------
