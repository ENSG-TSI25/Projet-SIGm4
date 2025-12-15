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
#include <iostream>

// === Chemin vers les données ===
static const std::string DATA_PATH = "/app/backend/data/required/";

class TransformationEngineTest : public ::testing::Test {
protected:    
    void SetUp() override {
        GDALAllRegister();
    }

    std::string createGpkgForEPSG(int epsgCode, const std::string& name, double epoch) {
        // Filepath
        std::string filename = "test_" + std::to_string(epsgCode) + ".gpkg";
        std::string path = "/tmp/" + filename;
        std::remove(path.c_str());
        
        GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
        GDALDataset* ds = driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
        
        // CRS
        OGRSpatialReference srs;
        srs.importFromEPSG(epsgCode);
        srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER); // Lon/X, Lat/Y

        // Coordinate type check
        bool isGeocentric = srs.IsGeocentric() || epsgCode == 4978 || epsgCode == 4936;
        bool isProjected = srs.IsProjected() || epsgCode == 3857 || epsgCode == 9794;
        
        // Layer initialization
        OGRLayer* layer = ds->CreateLayer("layer_0", &srs, wkbPoint25D, nullptr);
        
        // Epoch field
        OGRFieldDefn fieldEpoch("epoch", OFTReal);
        layer->CreateField(&fieldEpoch);

        // Feature initialization
        OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
        
        // CTest point coordinates
        OGRPoint point;
        
        if (isGeocentric) {
            // XYZ (approx ITRF/WGS84 ECEF)
            point.setX(4201713.4);
            point.setY(195245.7);
            point.setZ(4779483.5);
        }
        else if (isProjected) {
            if (epsgCode == 3857) { // WebMercator
                point.setX(296183.7); point.setY(6242493.5); point.setZ(0.0);
            }
            else if (epsgCode == 9794 || epsgCode == 2154) { // Lambert-93
                point.setX(659341.0); point.setY(6856536.0); point.setZ(0.0);
            }
            else { 
                // UTM or other projected CRS
                point.setX(500000.0); point.setY(5000000.0); point.setZ(0.0);
            }
        }
        else {
            // Geodetic (Degrees)
            point.setX(2.660663); 
            point.setY(48.808875); 
            point.setZ(100.0);
        }

        feature->SetGeometry(&point);
        feature->SetField("epoch", epoch);
        
        layer->CreateFeature(feature);
        OGRFeature::DestroyFeature(feature);
        GDALClose(ds);
        
        return path;
    }
};

// ---------------------------------------------------------------------------
// TEST 1 : Projected (WebMercator) -> Geodetic (ETRF2000)
// [Format: Metres -> Degrees]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Projected_To_Geodetic) {
    // Source : WebMercator (3857) - Metres
    std::string path = createGpkgForEPSG(3857, "WebMercator", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);    
    VectorLayer* layer = layers[0];
    ASSERT_NE(layer, nullptr);
    ASSERT_EQ(layer->getCrs(), "EPSG:3857");

    // Target : ETRF2000 Geo (EPSG:9067) - Degrees
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:9067");
    
    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification
    EXPECT_NEAR(p->getX(), 2.66066, 1e-4);
    EXPECT_NEAR(p->getY(), 48.80887, 1e-4);
}

// ---------------------------------------------------------------------------
// TEST 2 : Geodetic 2D (ITRF2014) -> Projected (Lambert-93)
// [Format: Degrees -> Metres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geodetic_To_Projected) {
    // Source : ITRF2014 2D (9000) - Degrees
    std::string path = createGpkgForEPSG(9000, "ITRF2014", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);
    VectorLayer* layer = layers.empty() ? nullptr : layers[0];
    ASSERT_NE(layer, nullptr);

    // Cible : RGF93 / Lambert-93 (EPSG:9794) - Metres
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:9794");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification
    EXPECT_NEAR(p->getX(), 675078.85, 10.0);
    EXPECT_NEAR(p->getY(), 6856587.17, 10.0);
}

// ---------------------------------------------------------------------------
// TEST 3 : Geodetic 3D (ITRF2014) -> Geodetic 3D (ETRF2000)
// [Format: Degrees -> Degrees]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geodetic3D_To_Geodetic3D) {
    // Source : ITRF2014 3D (7912)
    std::string path = createGpkgForEPSG(7912, "ITRF2014_3D", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);
    VectorLayer* layer = layers.empty() ? nullptr : layers[0];

    // Cible : ETRF2000 3D (7931)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:7931");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification
    EXPECT_NEAR(p->getX(), 2.66066, 1e-4);
    EXPECT_NEAR(p->getY(), 48.80887, 1e-4);
}

// ---------------------------------------------------------------------------
// TEST 4 : Geocentric (XYZ) -> Geodetic (LatLon)
// [Format: Metres -> Degrees]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geocentric_To_Geodetic) {
    // Source : WGS84 Geocentric (4978) - XYZ
    std::string path = createGpkgForEPSG(4978, "WGS84_XYZ", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);
VectorLayer* layer = layers.empty() ? nullptr : layers[0];
    
    // Cible : RGF93 LatLon (EPSG:4171)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:4171");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification
    EXPECT_NEAR(p->getX(), 2.66, 0.1);
    EXPECT_NEAR(p->getY(), 48.80, 0.1);
}

// ---------------------------------------------------------------------------
// TEST 5 : Geodetic (LatLon) (XYZ) -> Geocentric
// [Format: Metres -> Degrees]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geodetic_To_Geocentric) {
    // Source : RGF93 LatLon (EPSG:4171)
    std::string path = createGpkgForEPSG(4171, "RGF93_LatLon", 2025.0);
    DataManager dm; 
    auto layers = dm.loadVector(path);
    VectorLayer* layer = layers.empty() ? nullptr : layers[0];
    
    // Cible : WGS84 Geocentric (4978) - XYZ
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:4978");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification XYZ
    EXPECT_NEAR(p->getX(), 4203981.7, 10.0);
    EXPECT_NEAR(p->getY(), 195362.1, 10.0);
    EXPECT_NEAR(p->getZ(), 4776663.0, 10.0);
}

// ---------------------------------------------------------------------------
// TEST 6 : Projected -> Projected (Lambert-93 -> WebMercator)
// [Format: Metres -> Metres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Projected_To_Projected) {
    // Source : Lambert-93 (9794)
    std::string path = createGpkgForEPSG(9794, "Lambert93", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);
VectorLayer* layer = layers.empty() ? nullptr : layers[0];

    // Cible : WebMercator (3857)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:3857");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification
    EXPECT_NEAR(p->getX(), 272329.82, 10.0);
    EXPECT_NEAR(p->getY(), 6242280.42, 10.0);
}

// ---------------------------------------------------------------------------
// TEST 7 : Geocentric (XYZ) -> Geocentric (XYZ)
// [Format: Metres -> Metres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geocentric_To_Geocentric) {
    // Source : WGS84 Geocentric (4978) - XYZ
    std::string path = createGpkgForEPSG(4978, "WGS84_XYZ", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);
VectorLayer* layer = layers.empty() ? nullptr : layers[0];
    ASSERT_NE(layer, nullptr);
    ASSERT_EQ(layer->getCrs(), "EPSG:4978");

    // Target : ETRF2000 Geocentric (EPSG:4936)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:4936");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification 
    EXPECT_NEAR(p->getX(), 4201713.4, 10.0); 
    EXPECT_NEAR(p->getY(), 195245.7, 10.0);
    EXPECT_NEAR(p->getZ(), 4779483.5, 10.0);
    
}

// ---------------------------------------------------------------------------
// TEST 8 : Geocentric (XYZ) -> Projected (Lambert-93)
// [Format: Mètres -> Metres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geocentric_To_Projected) {
    // Source : WGS84 Geocentric (4978) - XYZ
    std::string path = createGpkgForEPSG(4978, "WGS84_XYZ", 2025.0);
    
    DataManager dm;
    auto layers = dm.loadVector(path);
VectorLayer* layer = layers.empty() ? nullptr : layers[0];

    // Target : RGF93 / Lambert-93 (EPSG:9794)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:9794");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification
    EXPECT_NEAR(p->getX(), 675083.16, 10.0);
    EXPECT_NEAR(p->getY(), 6860152.98, 10.0);
    EXPECT_NEAR(p->getZ(), 727.69, 10.0);
}
// ---------------------------------------------------------------------------
// TEST 9 : Projected (Lambert-93) -> Geocentric (XYZ)
// [Format: Metres -> Metres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Projected_To_Geocentric) {
    // Source : Lambert-93 (9794) - Metres
    std::string path = createGpkgForEPSG(9794, "Lambert93", 2025.0);
    DataManager dm;
    auto layers = dm.loadVector(path);
VectorLayer* layer = layers.empty() ? nullptr : layers[0];
    ASSERT_EQ(layer->getCrs(), "EPSG:9794");

    // Target : WGS84 Geocentric (EPSG:4978) - XYZ
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:4978");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification 
    EXPECT_NEAR(p->getX(), 4204722.53, 10.0);
    EXPECT_NEAR(p->getY(), 179639.87, 10.0);
    EXPECT_NEAR(p->getZ(), 4776495.43, 10.0);
}