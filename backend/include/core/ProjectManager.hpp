#pragma once
#include <vector>
#include <proj.h>
#include "Layer.hpp"
#include "Project.hpp"

class ProjectManager
{
private:
    Project project;

public:
    ProjectManager(Project project__);
    ~ProjectManager();

    std::vector<Layer> applyProjectParameters(Project projet_);
};
