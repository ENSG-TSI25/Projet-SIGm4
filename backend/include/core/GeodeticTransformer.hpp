#ifndef GEODETICTRANSFORMER_H
#define GEODETICTRANSFORMER_H

#pragma once
#include <string>
#include <proj.h>

class GeodeticTransformer {
public:
    GeodeticTransformer();
    ~GeodeticTransformer();

    struct Result {
        double x, y, z, t;
    };

    /// Transformations ITRF/ETRF/etc. dépendant du temps
    Result transformAtEpoch(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

    /// Déformation Mayotte : modèle JSON (+proj=defmodel)
    Result applyDefModel(
        double lon, double lat, double h, double t_epoch,
        const std::string& json_model_path,
        bool inverse = false);

    /// Déformation via grille : +proj=deformation
    Result applyGridDeformation(
        double x, double y, double z, double t_epoch,
        const std::string& grid_path,
        double ref_epoch);

    Result geocentricToGeodetic(
        double x, double y, double z, double t_epoch);

    Result geodeticToGeocentric(
        double lon, double lat, double h, double t_epoch);

    Result projectedToGeodetic(
        double x, double y, double h, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

    Result geodeticToProjected(
        double lon, double lat, double h, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

private:
    PJ_CONTEXT* ctx_;
};

#endif // GEODETICTRANSFORMER_H
