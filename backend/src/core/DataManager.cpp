#include <core/DataManager.hpp>
#include <iostream>



DataManager::DataManager() {}
DataManager::~DataManager() {}

VectorLayer *DataManager::loadVector(const std::string &chemin)
{
    // Ouverture du fichier GeoPackage
    GeoPackageReader reader(chemin);
    if (!reader.open())
        return nullptr;

    // Récupération des couches disponibles
    auto couches = reader.listLayers();
    if (couches.empty())
        return nullptr;

    // Lecture des métadonnées de la première couche
    auto metadata = reader.getLayerMetadata(couches[0]);
    auto layer = std::make_shared<VectorLayer>(couches[0], metadata.crs, metadata.referenceEpoch, metadata.coords_type);

    // Extraction et ajout des géométries dans la couche
    auto features = reader.extractFeatures(couches[0]);
    for (const auto &f : features)
    {
        auto geom = std::make_shared<Geometry4D>(f.geometry);
        layer->addGeometry(geom);
    }
    // Fermeture du fichier et stockage de la couche
    reader.close();
    vectorLayers.push_back(layer);

    return layer.get();
}
