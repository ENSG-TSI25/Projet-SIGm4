#pragma once

#include <core/GeodeticTransformer.hpp>
#include <core/VectorLayer.hpp>


class TransformationEngine : public GeodeticTransformer {
public:
    TransformationEngine() : GeodeticTransformer() {} ;
    ~TransformationEngine() = default;

    VectorLayer* transformLayerAtEpoch(VectorLayer& inputLayer, const std::string& epsg_dst);

    VectorLayer* applyDefModelLayer(VectorLayer& inputLayer, const std::string& json_model_path, bool inverse = false);

    VectorLayer* applyGridDeformationLayer(VectorLayer& inputLayer, const std::string& grid_path, double ref_epoch);

    Result transformPoint(
        OGRPoint *p, double t, const std::string &fmt_in, const std::string &fmt_out, const std::string &src_code, const std::string &dst_code);

    Result transformPointDefModel(
        OGRPoint *p, double t, const std::string &fmt_in, const std::string &model_path, int src_epsg, bool inverse);

    Result transformPointGrid(
        OGRPoint *p, double t, const std::string &fmt_in, const std::string &grid_path, int src_epsg, double ref_epoch);
};