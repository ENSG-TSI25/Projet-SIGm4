#include <core/Project.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>     
#include <stdexcept>
#include <cmath>     
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>

using namespace rapidjson;

Project::~Project() {
    layerList.clear();
}

void Project::addLayer(const Layer &l) {
    layerList.push_back(l);
}

void Project::rmLayer(const Layer &l) {
    std::vector<Layer>::iterator it = std::find(layerList.begin(), layerList.end(), l);
    if (it != layerList.end()) {
        layerList.erase(it);
    }
}

bool Project::save(const std::string& filepath) const {
    Document doc;
    doc.SetObject();
    Document::AllocatorType& allocator = doc.GetAllocator();
    
    // Add metadata
    doc.AddMember("version", "1.0", allocator);
    doc.AddMember("format", "SIGM4", allocator);
    doc.AddMember("name", Value(name.c_str(), allocator), allocator);
    doc.AddMember("crs", Value(crs.c_str(), allocator), allocator);
    doc.AddMember("epoch0", epoch0, allocator);
    
    // Add layers array
    Value layersArray(kArrayType);
    for (const auto& layer : layerList) {
        Value layerObj(kObjectType);
        layerObj.AddMember("name", Value(layer.getName().c_str(), allocator), allocator);
        layerObj.AddMember("crs", Value(layer.getCrs().c_str(), allocator), allocator);
        layerObj.AddMember("epoch", layer.getEpoch(), allocator);
        

        layerObj.AddMember("dataSource", Value(layer.getDataSource().c_str(), allocator), allocator);
        
        layersArray.PushBack(layerObj, allocator);
    }
    doc.AddMember("layers", layersArray, allocator);
    
    // Write to file
    FILE* fp = fopen(filepath.c_str(), "w");
    if (!fp) return false;
    
    char writeBuffer[65536];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    PrettyWriter<FileWriteStream> writer(os);
    writer.SetIndent(' ', 2);
    doc.Accept(writer);
    
    fclose(fp);
    std::cout << "Projet sauvegardé: " << filepath << std::endl;
    return true;
}


Project Project::load(const std::string& filepath) {
    FILE* fp = fopen(filepath.c_str(), "r");
    if (!fp) {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filepath);
    }
    
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    
    Document doc;
    doc.ParseStream(is);
    fclose(fp);
    
    if (doc.HasParseError()) {
        throw std::runtime_error("Erreur de parsing JSON");
    }
    
    // Extract data
    std::string name = doc["name"].GetString();
    std::string crs = doc["crs"].GetString();
    double epoch0 = doc["epoch0"].GetDouble();
    
    std::vector<Layer> layers;
    const Value& layersArray = doc["layers"];
    for (SizeType i = 0; i < layersArray.Size(); i++) {
        const Value& layerObj = layersArray[i];
        
        std::string layerName = layerObj["name"].GetString();
        std::string layerCrs = layerObj["crs"].GetString();
        double layerEpoch = layerObj["epoch"].GetDouble();
        
        std::string dataSource = "";
        if (layerObj.HasMember("dataSource")) {
            dataSource = layerObj["dataSource"].GetString();
        }
        
        layers.push_back(Layer(layerName, layerCrs, layerEpoch, dataSource));
    }
    
    std::cout << "Projet chargé: " << filepath << std::endl;
    std::cout << "  - Nom: " << name << std::endl;
    std::cout << "  - CRS: " << crs << std::endl;
    std::cout << "  - Époque: " << epoch0 << std::endl;
    std::cout << "  - Couches: " << layers.size() << std::endl;
    
    return Project(name, epoch0, crs, layers);
}