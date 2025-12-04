#include <core/Project.hpp>
#include <algorithm>

Project::~Project() {
    layerList.clear();
};

void Project::addLayer(const Layer& l) {
    layerList.push_back(l);
};

void Project::rmLayer(const Layer& l) {
    std::vector<Layer>::iterator it = std::find(layerList.begin(), layerList.end(), l);
    // Checks the layer is present in the list
    if (it != layerList.end()) { 
        layerList.erase(it);
    }
};
