#ifndef GEODETICTRANSFORMER_H
#define GEODETICTRANSFORMER_H

#pragma once
#include <string>
#include <proj.h>
#include <vector>
#include <map>

class GeodeticTransformer {
public:
    GeodeticTransformer();
    ~GeodeticTransformer();

    struct Result {
        double x, y, z, t;
    };

    Result transformCRS(
        double x, double y, double z,
        int epsg_src,
        int epsg_dst
    );

    struct CRSMetaData {
        std::string name;
        std::string type;
        double ref_epoch;  // O for non-time-dependent CRS
        std::vector<std::string> deformation_models;
    };

    // =============================================================
    // Registry of used CRS
    // =============================================================

    // Geometric 2D transformation registry
    static const std::map<int, CRSMetaData>& getRegistry2D() {
        static const std::map<int, CRSMetaData> reg = {
            // ITRF
            {9990,  {"ITRF2020", "Dynamic", 0, {}}},
            {9000,  {"ITRF2014", "Dynamic", 0, {"EuVeM2022"}}},
            {8999,  {"ITRF2008", "Dynamic", 0, {}}},
            {8998,  {"ITRF2005", "Dynamic", 0, {}}},
            {8987,  {"ITRF2000", "Dynamic", 0, {}}},
            // ETRF
            {10571, {"ETRF2020", "Dynamic", 0, {"deformation=0"}}},
            {9069,  {"ETRF2014", "Dynamic", 0, {"EuVeM2022", "deformation=0"}}},
            {9068,  {"ETRF2005", "Dynamic", 0, {"deformation=0"}}},
            {9067,  {"ETRF2000", "Dynamic", 0, {"EuVeM2022", "eur_nkg_nkgrf03vel_realigned.tif", "deformation=0"}}},
            // Static
            {9784,  {"RGF93v2b", "Static",  2019.0, {"deformation=0", "EuVeM2022"}}},
            {10673, {"RGM23",    "Static",  2023.75, {}}}
        };
        return reg;
    }

    // Geometric 3D transformation registry
    static const std::map<int, CRSMetaData>& getRegistry3D() {
        static const std::map<int, CRSMetaData> reg = {
            // ITRF
            {9989,  {"ITRF2020", "Dynamic", 0, {}}},
            {7912,  {"ITRF2014", "Dynamic", 0, {"EuVeM2022"}}},
            {7911,  {"ITRF2008", "Dynamic", 0, {}}},
            {7910,  {"ITRF2005", "Dynamic", 0, {}}},
            {7909,  {"ITRF2000", "Dynamic", 0, {}}},
            // ETRF
            {10570, {"ETRF2020", "Dynamic", 0, {"deformation=0"}}},
            {8403,  {"ETRF2014", "Dynamic", 0, {"EuVeM2022", "deformation=0"}}},
            {8399,  {"ETRF2005", "Dynamic", 0, {"deformation=0"}}},
            {7931,  {"ETRF2000", "Dynamic", 0, {"EuVeM2022", "eur_nkg_nkgrf03vel_realigned.tif", "deformation=0"}}},
            // Static
            {9783,  {"RGF93v2b", "Static",  2019.0, {"deformation=0", "EuVeM2022"}}},
            {10672, {"RGM23",    "Static",  2023.75, {}}}
        };
        return reg;
    }

    // Projected transformation registry
    static const std::map<int, CRSMetaData>& getRegistryProjected() {
        static const std::map<int, CRSMetaData> reg = {
            // RGF93 / Lambert-93
            {9794,  {"RGF93v2b", "Static", 2019.0, {"deformation=0", "EuVeM2022"}}},
            // RGM23 / UTM zone 38S
            {10674, {"RGM23",    "Static", 2023.75, {}}}
        };
        return reg;
    }

    /// Utility : Get CRS format from EPSG code
    static std::string getCRSFormat(int epsg_code);

    Result transformLinearAtEpoch(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

    Result transformGeodeticAtEpoch(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

    Result geocentricToGeodetic(
        double x, double y, double h, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

    Result geodeticToGeocentric(
        double x, double y, double h, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);
    
    Result projectedToGeodetic(
        double x, double y, double h, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);
    
    Result geodeticToProjected(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);
    
    Result projectedToGeocentric(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);
    
    Result geocentricToProjected(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst); 

    Result applyDefModelGeodetic(
        double lon_deg, double lat_deg, double h, double t_epoch,
        const std::string& json_model_path,
        bool inverse);

    Result applyDefModelGeocentric(
        double X, double Y, double Z, double t_epoch,
        const std::string& json_model_path,
        bool inverse);

    Result applyDefModelProjected(
        double E, double N, double H, double t_epoch,
        const std::string& json_model_path,
        int epsg_projected,
        bool inverse);

    Result applyGridDeformationGeodetic(
        double lon_deg, double lat_deg, double h, double t_epoch,
        const std::string& grid_path,
        double ref_epoch);

    Result applyGridDeformationGeocentric(
        double X, double Y, double Z, double t_epoch,
        const std::string& grid_path,
        double ref_epoch);

    GeodeticTransformer::Result applyGridDeformationProjected(
        double E, double N, double H, double t_epoch,
        const std::string& grid_path,
        int epsg_projected,
        double ref_epoch);
        
private:
    PJ_CONTEXT* ctx_;
};

#endif // GEODETICTRANSFORMER_H
