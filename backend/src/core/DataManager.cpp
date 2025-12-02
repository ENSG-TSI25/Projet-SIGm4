#include <core/DataManager.hpp>
#include <core/GeoPackageReader.hpp>
#include <iostream>

DataManager::DataManager() {}
DataManager::~DataManager() {}

VectorLayer *DataManager::chargerVecteur(const std::string &chemin)
{
    GeoPackageReader reader(chemin);
    if (!reader.open())
        return nullptr;

    auto couches = reader.listLayers();
    if (couches.empty())
        return nullptr;

    auto metadata = reader.getLayerMetadata(couches[0]);
    auto layer = std::make_shared<VectorLayer>(couches[0], metadata.crs, metadata.referenceEpoch);


    auto features = reader.extractFeatures(couches[0]);
    for (const auto &f : features)
    {
        layer->ajouterGeometrie(std::make_shared<Geometry4D>(f.geometry));
    }

    reader.close();
    vectorLayers.push_back(layer);
    return layer.get();
}