#pragma once
#include <vector>
#include <memory>
#include "Layer.hpp"
#include "Project.hpp"
#include "TransformationEngine.hpp"

class ProjectManager {
private:
    Project project;
    std::unique_ptr<TransformationEngine> engine;

public:
    explicit ProjectManager(const Project& project_);
    ~ProjectManager();

    // Applique les transformations du projet sur toutes les couches
    std::vector<std::shared_ptr<Layer>> applyProjectParameters();
    
    // Accesseurs
    const Project& getProject() const { return project; }
    void setProject(const Project& proj) { project = proj; }
};