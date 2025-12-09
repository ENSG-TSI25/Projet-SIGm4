#include <core/DataManager.hpp>
#include <core/GeoPackageReader.hpp>
#include <iostream>



DataManager::DataManager() {}
DataManager::~DataManager() {}

std::vector<VectorLayer*> DataManager::loadVector(const std::string &chemin)
{
    std::vector<VectorLayer*> loadedLayers; 

    // Ouverture du fichier GeoPackage
    GeoPackageReader reader(chemin);
    if (!reader.open())
        return loadedLayers; 

    // Récupération des couches disponibles
    auto couches = reader.listLayers();
    if (couches.empty())
        return loadedLayers;


     for (const auto& layerName : couches)
    {
        // Lecture des métadonnées de la couche
        auto metadata = reader.getLayerMetadata(layerName);
        auto layer = std::make_shared<VectorLayer>(layerName, metadata.crs, metadata.referenceEpoch);

        // Extraction et ajout des géométries dans la couche
        auto features = reader.extractFeatures(layerName);
        for (const auto &f : features)
        {
            auto geom = std::make_shared<Geometry4D>(f.geometry);
            layer->addGeometry(geom);
        }
        
        // Stockage de la couche
        vectorLayers.push_back(layer);
        loadedLayers.push_back(layer.get()); 
    }
    
    // Fermeture du fichier
    reader.close();

    return loadedLayers;
}
