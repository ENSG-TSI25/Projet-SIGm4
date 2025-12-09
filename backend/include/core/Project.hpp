#pragma once

#include <string>

#include <vector>

#include <fstream>

#include "Layer.hpp"

class Project
{

private:
    std::string name;

    std::string crs;

    double epoch0;

    std::vector<Layer> layerList;

public:
    Project(const std::string &name_, double epoch0_, const std::string &crs_ = "EPSG:4326", const std::vector<Layer> &layerList_ = {})

        : name(name_), crs(crs_), epoch0(epoch0), layerList(layerList_)
    {
    }

    ~Project();

    void addLayer(const Layer &l);

    void rmLayer(const Layer &l);

    std::string getName() const { return name; }

    std::string getCrs() const { return crs; }

    double getEpoch0() const { return epoch0; }

    std::vector<Layer> getLayers() const { return layerList; }

    void setName(const std::string &n) { name = n; }

    void setCrs(const std::string &c) { crs = c; }

    void setEpoch0(double e) { epoch0 = e; }
};