#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "Layer.hpp"

/**
 * @class Project
 * @brief Represents a GIS project
 * 
 * Contains project metadata and list of layers.
 */
class Project {
private:
    std::string name;
    std::string crs;
    double epoch0;
    std::vector<Layer> layerList;

public:
    /**
     * @brief Constructor
     * @param name_ Project name
     * @param epoch0_ Reference epoch
     * @param crs_ Coordinate system
     * @param layerList_ Initial layers
     */
    Project(const std::string &name_, double epoch0_, const std::string &crs_ = "EPSG:4326", const std::vector<Layer> &layerList_ = {})
        : name(name_), crs(crs_), epoch0(epoch0), layerList(layerList_) {}

    ~Project();

    /**
     * @brief Adds a layer to the project
     * @param l Layer to add
     */
    void addLayer(const Layer &l);

    /**
     * @brief Removes a layer from the project
     * @param l Layer to remove
     */
    void rmLayer(const Layer &l);

    std::string getName() const { return name; }
    std::string getCrs() const { return crs; }
    double getEpoch0() const { return epoch0; }
    std::vector<Layer> getLayers() const { return layerList; }

    void setName(const std::string &n) { name = n; }
    void setCrs(const std::string &c) { crs = c; }
    void setEpoch0(double e) { epoch0 = e; }
};