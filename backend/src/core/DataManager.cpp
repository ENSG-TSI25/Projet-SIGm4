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


        layer->setDataSource(chemin);

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


RasterLayer* DataManager::loadRaster(const std::string& chemin) {
    std::cout << "DEBUG: Opening file: " << chemin << std::endl;
    
    GeoPackageReader reader(chemin);
    if (!reader.open()) {
        std::cout << "DEBUG: Failed to open file" << std::endl;
        return nullptr;
    }
    
    std::vector<std::string> rasterNames = {"raster_data", "tiles", "imagery", "ortho", "dem"};
    
    for (const auto& name : rasterNames) {
        std::cout << "DEBUG: Trying raster table: " << name << std::endl;
        if (reader.isRasterLayer(name)) {
            std::cout << "DEBUG: Found raster layer: " << name << std::endl;
            
            auto metadata = reader.extractRasterMetadata(name);
            
            auto layer = std::make_shared<RasterLayer>(
                metadata.name, metadata.crs, metadata.referenceEpoch, chemin);
            
            layer->setDimensions(metadata.width, metadata.height);
            layer->setGeoTransform(metadata.geoTransform);
            layer->setFilePath(metadata.layerPath);  
            
            auto emprise = std::make_shared<Geometry4D>(
                reader.extractRasterExtent(name, metadata.referenceEpoch));
            layer->setEmprise(emprise);
            
            rasterLayers.push_back(layer);
            reader.close();
            return layer.get();
        }
    }
    
    auto couches = reader.listLayers();
    std::cout << "DEBUG: Found " << couches.size() << " layers from listLayers()" << std::endl;
    
    for (const auto& layerName : couches) {
        std::cout << "DEBUG: Checking layer: " << layerName << std::endl;
        
        if (reader.isRasterLayer(layerName)) {
            std::cout << "DEBUG: Found raster layer: " << layerName << std::endl;
            
            auto metadata = reader.extractRasterMetadata(layerName);
            
            auto layer = std::make_shared<RasterLayer>(
                metadata.name, metadata.crs, metadata.referenceEpoch, chemin);
            
            layer->setDimensions(metadata.width, metadata.height);
            layer->setGeoTransform(metadata.geoTransform);
            layer->setFilePath(metadata.layerPath); 
            
            auto emprise = std::make_shared<Geometry4D>(
                reader.extractRasterExtent(layerName, metadata.referenceEpoch));
            layer->setEmprise(emprise);
            
            rasterLayers.push_back(layer);
            reader.close();
            return layer.get();
        }
    }
    
    std::cout << "DEBUG: No raster layer found in file" << std::endl;
    reader.close();
    return nullptr;
}