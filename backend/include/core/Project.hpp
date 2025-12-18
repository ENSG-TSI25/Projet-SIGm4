#pragma once
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include "Layer.hpp"

class Project {
private:
    std::string name;
    std::string crs;
    double epoch0;
    std::string deformationModel;
    std::vector<std::shared_ptr<Layer>> layerList;

public:
    Project(const std::string &name_, double epoch0_, const std::string &crs_ = "EPSG:4326", 
            const std::vector<std::shared_ptr<Layer>> &layerList_ = {})
        : name(name_), crs(crs_), epoch0(epoch0_), layerList(layerList_), deformationModel("") {}
    
    ~Project();
    
    void addLayer(std::shared_ptr<Layer> l);
    void rmLayer(const std::shared_ptr<Layer>& l);
    
    std::string getName() const { return name; }
    std::string getCrs() const { return crs; }
    double getEpoch0() const { return epoch0; }
    std::string getDeformationModel() const { return deformationModel; }
    std::vector<std::shared_ptr<Layer>> getLayers() const { return layerList; }
    
    void setName(const std::string &n) { name = n; }
    void setCrs(const std::string &c) { crs = c; }
    void setEpoch0(double e) { epoch0 = e; }
    void setDeformationModel(const std::string &model) { deformationModel = model; }
     
    bool save(const std::string &filepath) const;
    static Project load(const std::string &filepath);
};