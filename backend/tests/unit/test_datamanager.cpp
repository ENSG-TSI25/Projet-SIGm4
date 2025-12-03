#include <gtest/gtest.h>
#include <core/DataManager.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_geometry.h>
#include <fstream>

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

// Chargement fichier valide
TEST_F(DataManagerTest, ChargerVecteurValid) {
    std::string path = createTestGpkg();
    DataManager dm;
    
    VectorLayer* layer = dm.chargerVecteur(path);
    
    ASSERT_NE(layer, nullptr);
    EXPECT_FALSE(layer->getGeometries().empty());
    
    std::remove(path.c_str());
}

// Fichier inexistant
TEST_F(DataManagerTest, ChargerVecteurInvalid) {
    DataManager dm;
    VectorLayer* layer = dm.chargerVecteur("/tmp/nonexistent.gpkg");
    EXPECT_EQ(layer, nullptr);
}

// Fichier vide
TEST_F(DataManagerTest, ChargerVecteurEmpty) {
    std::string path = "/tmp/empty.gpkg";
    
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
    GDALDataset* ds = driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    GDALClose(ds);
    
    DataManager dm;
    VectorLayer* layer = dm.chargerVecteur(path);
    
    EXPECT_EQ(layer, nullptr);
    std::remove(path.c_str());
}