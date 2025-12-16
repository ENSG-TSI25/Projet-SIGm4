#include <gtest/gtest.h>
#include <core/ProjectManager.hpp>
#include <core/Project.hpp>
#include <core/VectorLayer.hpp>
#include <core/RasterLayer.hpp>
#include <core/Geometry4D.hpp>
#include <gdal/ogr_geometry.h>
#include <gdal/gdal.h>
#include <memory>

class ProjectManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        GDALAllRegister();
    }
    
    std::shared_ptr<VectorLayer> createTestVectorLayer(
        const std::string& name, 
        const std::string& crs,
        double x, double y, double z = 0.0, double t = 2025.0) {
        
        auto layer = std::make_shared<VectorLayer>(name, crs, t, "geodetic");
        
        auto* point = new OGRPoint(x, y, z);
        auto geom = std::make_shared<Geometry4D>(point);
        geom->setT(t);
        layer->addGeometry(geom);
        
        return layer;
    }
};

TEST_F(ProjectManagerTest, Constructor) {
    Project proj("Test", 2025.0, "EPSG:4326");
    ProjectManager pm(proj);
    EXPECT_EQ(pm.getProject().getName(), "Test");
}

TEST_F(ProjectManagerTest, EmptyProject) {
    Project proj("Empty", 2025.0, "EPSG:4326");
    ProjectManager pm(proj);
    auto result = pm.applyProjectParameters();
    EXPECT_TRUE(result.empty());
}

TEST_F(ProjectManagerTest, LayerAlreadyInTargetCRS) {
    auto layer = createTestVectorLayer("TestLayer", "EPSG:4326", 2.5, 48.8);
    
    Project proj("Test", 2025.0, "EPSG:4326");
    proj.addLayer(layer);
    
    ProjectManager pm(proj);
    auto result = pm.applyProjectParameters();
    
    EXPECT_TRUE(result.empty());
}


TEST_F(ProjectManagerTest, TransformVectorLayer) {
    auto layer = createTestVectorLayer("Layer1", "EPSG:4326", 2.5, 48.8);
    
    Project proj("Test", 2025.0, "EPSG:2154");
    proj.addLayer(layer);
    
    ProjectManager pm(proj);
    auto result = pm.applyProjectParameters();
    
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(result[0].find("Layer1_transformed_2154") != std::string::npos);
}

TEST_F(ProjectManagerTest, MultipleLayers) {
    auto layer1 = createTestVectorLayer("L1", "EPSG:4326", 2.5, 48.8);
    auto layer2 = createTestVectorLayer("L2", "EPSG:3857", 278000, 6250000);
    
    Project proj("Test", 2025.0, "EPSG:2154");
    proj.addLayer(layer1);
    proj.addLayer(layer2);
    
    ProjectManager pm(proj);
    auto result = pm.applyProjectParameters();
    
    ASSERT_EQ(result.size(), 2);
    EXPECT_TRUE(result[0].find("transformed_2154") != std::string::npos);
    EXPECT_TRUE(result[1].find("transformed_2154") != std::string::npos);
}
TEST_F(ProjectManagerTest, RasterLayerSkipped) {
    auto rasterLayer = std::make_shared<RasterLayer>("Raster1", "EPSG:4326", 2025.0);
    
    Project proj("Test", 2025.0, "EPSG:2154");
    proj.addLayer(rasterLayer);
    
    ProjectManager pm(proj);
    auto result = pm.applyProjectParameters();
    
    EXPECT_TRUE(result.empty());  // Raster not supported
}

TEST_F(ProjectManagerTest, MixedLayers) {
    auto vecLayer = createTestVectorLayer("Vector", "EPSG:4326", 2.5, 48.8);
    auto rasLayer = std::make_shared<RasterLayer>("Raster", "EPSG:4326", 2025.0);
    
    Project proj("Test", 2025.0, "EPSG:2154");
    proj.addLayer(vecLayer);
    proj.addLayer(rasLayer);
    
    ProjectManager pm(proj);
    auto result = pm.applyProjectParameters();
    
    ASSERT_EQ(result.size(), 1);  // Only vector layer
    EXPECT_TRUE(result[0].find("Vector_transformed_2154") != std::string::npos);
}