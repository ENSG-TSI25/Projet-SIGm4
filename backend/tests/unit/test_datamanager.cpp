#include <gtest/gtest.h>
#include <core/DataManager.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>
#include <fstream>

// Initialise GDAL avant chaque test
class DataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        GDALAllRegister();
    }
    
    // Crée un GeoPackage de test temporaire
    std::string createTestGpkg() {
        std::string path = "/tmp/test_data.gpkg";
        std::remove(path.c_str());
        
        GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
        GDALDataset* ds = driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
        
        OGRLayer* layer = ds->CreateLayer("test_layer", nullptr, wkbPoint, nullptr);
        OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
        OGRPoint point(2.0, 48.0);
        feature->SetGeometry(&point);
        layer->CreateFeature(feature);
        OGRFeature::DestroyFeature(feature);
        
        GDALClose(ds);
        return path;
    }
};

// Constructeur
TEST_F(DataManagerTest, Constructor) {
    DataManager dm;
    SUCCEED();
}

TEST_F(DataManagerTest, loadVectorValid) {
    std::string path = createTestGpkg();
    DataManager dm;
    
    auto layers = dm.loadVector(path);
    
    ASSERT_FALSE(layers.empty());  
    ASSERT_NE(layers[0], nullptr);  
    EXPECT_FALSE(layers[0]->getGeometries().empty());  
    EXPECT_FALSE(layers[0]->getEWKT().empty());  
    
    std::remove(path.c_str());
}

TEST_F(DataManagerTest, loadVectorInvalid) {
    DataManager dm;
    
    auto layers = dm.loadVector("/tmp/nonexistent.gpkg");
    
    EXPECT_TRUE(layers.empty());  
}

TEST_F(DataManagerTest, loadVectorEmpty) {
    std::string path = "/tmp/empty.gpkg";
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
    GDALDataset* ds = driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    GDALClose(ds);
    
    DataManager dm;
    
    auto layers = dm.loadVector(path);
    
    EXPECT_TRUE(layers.empty());  
    
    std::remove(path.c_str());
}



// Test loadRaster invalide
TEST_F(DataManagerTest, loadRasterInvalid) {
    DataManager dm;
    RasterLayer* raster = dm.loadRaster("/tmp/nonexistent.gpkg");
    EXPECT_EQ(raster, nullptr);
}