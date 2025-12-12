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

        // Détection du type pour savoir quelles coordonnées injecter
        bool isProjected = srs.IsProjected();
        bool isGeocentric = srs.IsGeocentric();
        
        // Layer initialization
        OGRLayer* layer = ds->CreateLayer("layer_0", &srs, wkbPoint25D, nullptr);
        
        // Epoch field
        OGRFieldDefn fieldEpoch("epoch", OFTReal);
        layer->CreateField(&fieldEpoch);

        // Feature
        OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());
        
        // COORDONNÉES DE TEST (Champs-sur-Marne)
        OGRPoint point;
        
        if (isGeocentric) {
            // XYZ (approx ITRF/WGS84 ECEF)
            // X=4201713, Y=195245, Z=4779483
            point.setX(4201713.4);
            point.setY(195245.7);
            point.setZ(4779483.5);
        }
        else if (isProjected) {
            // Coordonnées approximatives selon le système pour éviter d'être hors zone
            if (epsgCode == 3857) { // WebMercator
                point.setX(296183.7); point.setY(6242493.5); point.setZ(0.0);
            }
            else if (epsgCode == 9794 || epsgCode == 2154) { // Lambert-93
                point.setX(659341.0); point.setY(6856536.0); point.setZ(0.0);
            }
            else { 
                // Fallback UTM ou autre (Mètres génériques)
                point.setX(500000.0); point.setY(5000000.0); point.setZ(0.0);
            }
        }
        else {
            // Géographique (Degrés)
            // Lon ~ 2.66, Lat ~ 48.81
            point.setX(2.660663); 
            point.setY(48.808875); 
            point.setZ(100.0); // Hauteur ellipsoïdale
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
// [Format: Mètres -> Degrés]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Projected_To_Geodetic) {
    // 1. Source : WebMercator (3857) - Mètres
    std::string path = createGpkgForEPSG(3857, "WebMercator", 2025.0);
    
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);
    ASSERT_NE(layer, nullptr);
    ASSERT_EQ(layer->getCrs(), "EPSG:3857");

    // 2. Cible : ETRF2000 Geo (EPSG:9067) - Degrés
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:9067");
    
    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification (Lon ~2.66, Lat ~48.81)
    EXPECT_NEAR(p->getX(), 2.66066, 1e-4);
    EXPECT_NEAR(p->getY(), 48.80887, 1e-4);
}

// ---------------------------------------------------------------------------
// TEST 2 : Geodetic 2D (ITRF2014) -> Projected (Lambert-93)
// [Format: Degrés -> Mètres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geodetic_To_Projected) {
    // 1. Source : ITRF2014 2D (9000) - Degrés
    std::string path = createGpkgForEPSG(9000, "ITRF2014", 2025.0);
    
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);
    ASSERT_NE(layer, nullptr);

    // 2. Cible : RGF93 / Lambert-93 (EPSG:9794) - Mètres
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:9794");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification Lambert-93 (approx 659k, 6856k)
    EXPECT_NEAR(p->getX(), 675078.85, 10.0);
    EXPECT_NEAR(p->getY(), 6856587.17, 10.0);
}

// ---------------------------------------------------------------------------
// TEST 3 : Geodetic 3D (ITRF2014) -> Geodetic 3D (ETRF2000)
// [Format: Degrés -> Degrés]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geodetic3D_To_Geodetic3D) {
    // 1. Source : ITRF2014 3D (7912)
    std::string path = createGpkgForEPSG(7912, "ITRF2014_3D", 2025.0);
    
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);

    // 2. Cible : ETRF2000 3D (7931)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:7931");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification : Doit rester en degrés
    EXPECT_NEAR(p->getX(), 2.66066, 1e-4);
    EXPECT_NEAR(p->getY(), 48.80887, 1e-4);
    // Le Z et les coords bougent légèrement à cause du changement de plaque tectonique
}

// ---------------------------------------------------------------------------
// TEST 4 : Geocentric (XYZ) -> Geodetic (LatLon)
// [Format: Mètres -> Degrés]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Geocentric_To_Geodetic) {
    // 1. Source : WGS84 Geocentric (4978) - XYZ
    // Note: createGpkgForEPSG va générer des coordonnées XYZ ~4M
    std::string path = createGpkgForEPSG(4978, "WGS84_XYZ", 2025.0);
    
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);
    
    // 2. Cible : RGF93 LatLon (EPSG:4171)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:4171");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification : On doit retrouver nos degrés
    EXPECT_NEAR(p->getX(), 2.66, 0.1); // Tolérance large car XYZ approximatif généré
    EXPECT_NEAR(p->getY(), 48.80, 0.1);
}

// ---------------------------------------------------------------------------
// TEST 5 : Projected -> Projected (Lambert-93 -> WebMercator)
// [Format: Mètres -> Mètres]
// ---------------------------------------------------------------------------
TEST_F(TransformationEngineTest, Transform_Projected_To_Projected) {
    // 1. Source : Lambert-93 (9794)
    std::string path = createGpkgForEPSG(9794, "Lambert93", 2025.0);
    
    DataManager dm;
    VectorLayer* layer = dm.loadVector(path);

    // 2. Cible : WebMercator (3857)
    TransformationEngine engine;
    VectorLayer* res = engine.transformLayerAtEpoch(*layer, "EPSG:3857");

    OGRPoint* p = res->getGeometries()[0]->getGeometry()->toPoint();

    // Verification WebMercator
    EXPECT_NEAR(p->getX(), 272329.82, 10.0);
    EXPECT_NEAR(p->getY(), 6242280.42, 10.0);
}