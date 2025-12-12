#include <gtest/gtest.h>
#include <core/TransformationEngine.hpp>
#include <core/DataManager.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_api.h>
#include <gdal/ogr_spatialref.h>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <string>

// === Chemin vers les données ===
static const std::string DATA_PATH = "/app/backend/data/required/";

class TransformationEngineTest : public ::testing::Test {
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
        
        // CRS
        OGRSpatialReference srs;
        srs.importFromEPSG(3857); 
        srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER); //(Longitude/X, Latitude/Y)

        OGRLayer* layer = ds->CreateLayer("test_layer", &srs, wkbPoint, nullptr);

        // Date
        OGRFieldDefn fieldEpoch("epoch", OFTReal);
        if (layer->CreateField(&fieldEpoch) != OGRERR_NONE) {
            GDALClose(ds);
            throw std::runtime_error("Error when creating epoch field");
        }

        OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
        OGRPoint point(48.808889, 2.660612, 0);
        feature->SetGeometry(&point);
        feature->SetField("epoch", 2025.0);
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
    std::string path = createTestGpkg();
    DataManager dm;
    auto layers = dm.loadVector(path);
    VectorLayer* layer = layers.empty() ? nullptr : layers[0];
    ASSERT_NE(layer, nullptr);
    ASSERT_EQ(layer->getCrs(), "EPSG:3857");
    
    TransformationEngine engine;
    VectorLayer* transformedLayer = engine.transformLayerAtEpoch(*layer, "9794");
    ASSERT_NE(transformedLayer, nullptr);

    auto geometries = transformedLayer->getGeometries();
    ASSERT_FALSE(geometries.empty());
    
    auto geom = geometries[0];
    OGRGeometry* ogrGeom = geom->getGeometry();
    ASSERT_NE(ogrGeom, nullptr);
    OGRPoint* point = ogrGeom->toPoint();
    ASSERT_NE(point, nullptr);

    // Tolerances : 
    double DEG_EPS = 1e-2; // OUCH
    double H_EPS = 1e-3;

    // EXPECT_NEAR(point->getX(), 909839.98, DEG_EPS);
    
    // EXPECT_NEAR(point->getY(), 253596.48, DEG_EPS);
    
    // EXPECT_NEAR(point->getZ(), 0, H_EPS);
    // EXPECT_EQ(geom->getT(), 2025);
    
}