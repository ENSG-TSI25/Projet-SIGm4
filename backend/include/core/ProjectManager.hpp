#pragma once
#include <vector>
#include <memory>
#include <string>
#include "Layer.hpp"
#include "Project.hpp"
#include "TransformationEngine.hpp"
#include <gdal/gdal.h>
#include <gdal/ogrsf_frmts.h>

class VectorLayer;

class ProjectManager {
private:
    Project project;
    std::unique_ptr<TransformationEngine> engine;
    
    // Helper methods for file generation
    std::string getCurrentTimestamp();
    std::string generateOutputFilename(
        const std::string& layerName,
        const std::string& targetEpsg,
        const std::string& timestamp);
    void updateGeoPackageWithTransformedData(
        const std::string& gpkgPath,
        const std::shared_ptr<VectorLayer>& layer,
        const std::string& targetCrs);
    void createGeoPackageFromLayer(
        const std::string& gpkgPath,
        const std::shared_ptr<VectorLayer>& layer,
        const std::string& targetCrs);

public:
    explicit ProjectManager(const Project& project_);
    ~ProjectManager();
    
    // Apply transformations and return file paths
    std::vector<std::string> applyProjectParameters();
    
    // Accessors
    const Project& getProject() const { return project; }
    void setProject(const Project& proj) { project = proj; }
};