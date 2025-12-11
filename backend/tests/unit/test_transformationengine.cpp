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
};

// Builder
TEST_F(TransformationEngineTest, Constructor) {
    TransformationEngine engine;
    SUCCEED();
}

// Loading Valid File
TEST_F(TransformationEngineTest, transformLayerAtEpoch) {
    std::string path = "/app/data/test_data.gpkg";
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);
    ASSERT_NE(layer, nullptr);
    ASSERT_EQ(layer->getCrs(), "EPSG:10674");
    
    TransformationEngine engine;
    VectorLayer* transformedLayer = engine.transformLayerAtEpoch(*layer, "4326");
    ASSERT_NE(transformedLayer, nullptr);

    auto geometries = transformedLayer->getGeometries();
    ASSERT_FALSE(geometries.empty());
    
    auto geom = geometries[0];
    OGRGeometry* ogrGeom = geom->getGeometry();
    ASSERT_NE(ogrGeom, nullptr);
    OGRPoint* point = ogrGeom->toPoint();
    ASSERT_NE(point, nullptr);

    // Tolerances : 
    double DEG_EPS = 1e-4; 
    double H_EPS = 1e-3;

    EXPECT_NEAR(point->getX(), 45.1232, DEG_EPS);
    
    EXPECT_NEAR(point->getY(), -12.7136, DEG_EPS);
    
    EXPECT_NEAR(point->getZ(), 0, H_EPS);
    EXPECT_EQ(geom->getT(), 1.181779200e+09);
    
}