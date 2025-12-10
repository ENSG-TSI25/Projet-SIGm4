#include <core/Project.hpp>
#include <algorithm>

/**
 * @file Project.cpp
 * @brief Implementation of Project class
 * 
 * Contains project management operations.
 */

/**
 * @brief Destructor - clears layer list
 */
Project::~Project()
{
    layerList.clear();
};

/**
 * @brief Adds a layer to the project
 * @param l Layer to add
 */
void Project::addLayer(const Layer &l)
{
    layerList.push_back(l);
};

/**
 * @brief Removes a layer from the project
 * @param l Layer to remove
 */
void Project::rmLayer(const Layer &l)
{
    std::vector<Layer>::iterator it = std::find(layerList.begin(), layerList.end(), l);

    if (it != layerList.end())
    {
        layerList.erase(it);
    }
};