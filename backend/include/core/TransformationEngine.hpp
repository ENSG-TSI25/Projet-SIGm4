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
};