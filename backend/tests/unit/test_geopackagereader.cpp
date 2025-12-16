#include <gtest/gtest.h>
#include <core/GeoPackageReader.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogrsf_frmts.h>

class GeoPackageReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        GDALAllRegister();
    }
    
    // Crée un GeoPackage de test avec timestamp
    std::string createTestGpkg() {
        std::string path = "/tmp/test_reader.gpkg";
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

// Constructeur
TEST_F(GeoPackageReaderTest, Constructor) {
    GeoPackageReader reader("/tmp/test.gpkg");
    SUCCEED();
}

// Ouverture fichier valide
TEST_F(GeoPackageReaderTest, OpenValid) {
    std::string path = createTestGpkg();
    GeoPackageReader reader(path);
    EXPECT_TRUE(reader.open());
    reader.close();
    std::remove(path.c_str());
}

// Ouverture fichier inexistant
TEST_F(GeoPackageReaderTest, OpenInvalid) {
    GeoPackageReader reader("/tmp/nonexistent.gpkg");
    EXPECT_FALSE(reader.open());
}

// Liste des couches
TEST_F(GeoPackageReaderTest, ListLayers) {
    std::string path = createTestGpkg();
    GeoPackageReader reader(path);
    reader.open();
    
    auto layers = reader.listLayers();
    ASSERT_EQ(layers.size(), 1);
    EXPECT_EQ(layers[0], "test_layer");
    
    reader.close();
    std::remove(path.c_str());
}

// Métadonnées de couche
TEST_F(GeoPackageReaderTest, GetLayerMetadata) {
    std::string path = createTestGpkg();
    GeoPackageReader reader(path);
    reader.open();
    
    auto metadata = reader.getLayerMetadata("test_layer");
    EXPECT_EQ(metadata.name, "test_layer");
    EXPECT_EQ(metadata.featureCount, 1);
    EXPECT_EQ(metadata.geometryType, wkbPoint);
    
    reader.close();
    std::remove(path.c_str());
}

// Extraction features
TEST_F(GeoPackageReaderTest, ExtractFeatures) {
    std::string path = createTestGpkg();
    GeoPackageReader reader(path);
    reader.open();
    
    auto features = reader.extractFeatures("test_layer");
    ASSERT_EQ(features.size(), 1);
    EXPECT_DOUBLE_EQ(features[0].timestamp, 2024.5);
    
    reader.close();
    std::remove(path.c_str());
}




// Test isRasterLayer
TEST_F(GeoPackageReaderTest, IsRasterLayer) {
    std::string path = "/tmp/test_raster.gpkg";
    std::remove(path.c_str());
    
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
    GDALDataset* ds = driver->Create(path.c_str(), 10, 10, 1, GDT_Byte, nullptr);
    
    double gt[6] = {100.0, 1.0, 0.0, 200.0, 0.0, -1.0};
    ds->SetGeoTransform(gt);
    GDALClose(ds);
    
    GeoPackageReader reader(path);
    reader.open();
    EXPECT_TRUE(reader.isRasterLayer("test_raster"));
    reader.close();
    std::remove(path.c_str());
}

// Test extractRasterMetadata
TEST_F(GeoPackageReaderTest, ExtractRasterMetadata) {
    std::string path = "/tmp/test_raster.gpkg";
    std::remove(path.c_str());
    
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GPKG");
    GDALDataset* ds = driver->Create(path.c_str(), 10, 10, 1, GDT_Byte, nullptr);
    double gt[6] = {100.0, 1.0, 0.0, 200.0, 0.0, -1.0};
    ds->SetGeoTransform(gt);
    GDALClose(ds);
    
    GeoPackageReader reader(path);
    reader.open();
    auto metadata = reader.extractRasterMetadata("test_raster");
    
    EXPECT_EQ(metadata.width, 10);
    EXPECT_EQ(metadata.height, 10);
    EXPECT_DOUBLE_EQ(metadata.geoTransform[0], 100.0);
    
    reader.close();
    std::remove(path.c_str());
}