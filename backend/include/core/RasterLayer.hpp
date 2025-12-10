// backend/include/core/RasterLayer.hpp
#pragma once
#include <core/Layer.hpp>
#include <core/Geometry4D.hpp>
#include <memory>

class RasterLayer : public Layer {
private:
    std::shared_ptr<Geometry4D> emprise;
    std::string filePath;
    double geoTransform[6];
    int width, height;

public:
    RasterLayer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", 
                double epoque_ = 0.0)
        : Layer(nom_, crs_, epoque_), width(0), height(0) {}
    
    void setEmprise(std::shared_ptr<Geometry4D> bbox) { emprise = bbox; }
    void setFilePath(const std::string& path) { filePath = path; }
    void setGeoTransform(const double* gt) { 
        std::copy(gt, gt + 6, geoTransform); 
    }
    void setDimensions(int w, int h) { width = w; height = h; }
    
    std::shared_ptr<Geometry4D> getEmprise() const { return emprise; }
    std::string getFilePath() const { return filePath; }
    const double* getGeoTransform() const { return geoTransform; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};