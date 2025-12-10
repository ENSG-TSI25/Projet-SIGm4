#include <gtest/gtest.h>
#include <core/TransformationEngine.hpp>
#include <core/DataManager.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>

// === Chemin vers les données ===
static const std::string DATA_PATH = "/app/backend/data/required/";

class TransformationEngineTest : public ::testing::Test {
protected:    
    void SetUp() override {
        GDALAllRegister();
    }
    
    // Crée un GeoPackage de test avec timestamp
    std::string createTestGpkg() {
        std::string path = "/tmp/test_data.gpkg";
        std::remove(path.c_str());
        
        GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
        GDALDataset* ds = driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
        
        OGRLayer* layer = ds->CreateLayer("test_layer", nullptr, wkbPoint, nullptr);
        
        // Ajoute un champ timestamp
        OGRFieldDefn timeField("timestamp", OFTReal);
        layer->CreateField(&timeField);
        
        // Ajoute une feature avec timestamp
        OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
        OGRPoint point(2.0, 48.0);
        feature->SetGeometry(&point);
        feature->SetField("timestamp", 2024.5);
        layer->CreateFeature(feature);
        OGRFeature::DestroyFeature(feature);
        
        GDALClose(ds);
        return path;
    }
};

// Builder
TEST_F(TransformationEngineTest, Constructor) {
    TransformationEngine engine;
    SUCCEED();
}

// Loading Valid File
TEST_F(TransformationEngineTest, transformLayerAtEpoch) {
    //std::string path = createTestGpkg();
    std::string path = "/app/data/test_data.gpkg";
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);
    ASSERT_NE(layer, nullptr);
    ASSERT_EQ(layer->getCrs(), "EPSG:10674");
    
    TransformationEngine engine;
    VectorLayer* transformedLayer = engine.transformLayerAtEpoch(*layer, "4326");
    
    auto geometries = transformedLayer->getGeometries();
    ASSERT_FALSE(geometries.empty());
    
    auto geom = geometries[0];
    OGRGeometry* ogrGeom = geom->getGeometry();
    OGRPoint* point = ogrGeom->toPoint();
    
    EXPECT_DOUBLE_EQ(point->getX(), -4893.69);
    EXPECT_DOUBLE_EQ(point->getY(), -1618.46);
    EXPECT_DOUBLE_EQ(point->getZ(), 0);
    EXPECT_DOUBLE_EQ(geom->getT(),  1.18178e+09);
    
    std::remove(path.c_str());
}