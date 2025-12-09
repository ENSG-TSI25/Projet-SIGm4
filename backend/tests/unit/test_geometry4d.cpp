#include <gtest/gtest.h>
#include <core/Geometry4D.hpp>
#include <gdal/ogr_geometry.h>
#include <gdal/ogr_api.h>
#include <gdal/gdal.h>

// Fixture pour initialiser GDAL avant chaque test
class Geometry4DTest : public ::testing::Test {
protected:
    void SetUp() override {
        GDALAllRegister();
    }
};

// Teste le constructeur par défaut
TEST_F(Geometry4DTest, DefaultConstructor) {
    Geometry4D geom;
    EXPECT_EQ(geom.getGeometry(), nullptr);
}

// Teste la création avec un point et l'activation de la dimension M
TEST_F(Geometry4DTest, ConstructorWithPoint) {
    OGRPoint* point = new OGRPoint(2.0, 48.0, 100.0);
    Geometry4D geom(point);
    ASSERT_NE(geom.getGeometry(), nullptr);
    EXPECT_TRUE(geom.getGeometry()->IsMeasured());
}

// Teste le stockage du temps dans M
TEST_F(Geometry4DTest, SetAndGetTimestamp) {
    OGRPoint* point = new OGRPoint(2.0, 48.0, 100.0);
    Geometry4D geom(point);
    geom.setT(2024.5);
    EXPECT_DOUBLE_EQ(geom.getT(), 2024.5);
}

// Teste l'affectation du SRID
TEST_F(Geometry4DTest, SetAndGetSRID) {
    OGRPoint* point = new OGRPoint(2.0, 48.0);
    Geometry4D geom(point);
    geom.setSRID(2154);
    EXPECT_EQ(geom.getSRID(), 2154);
}

// Teste la copie d'objet
TEST_F(Geometry4DTest, CopyConstructor) {
    OGRPoint* point = new OGRPoint(2.0, 48.0);
    Geometry4D geom1(point);
    geom1.setT(2024.0);
    Geometry4D geom2(geom1);
    EXPECT_DOUBLE_EQ(geom2.getT(), 2024.0);
}

// Teste l'opérateur d'affectation
TEST_F(Geometry4DTest, AssignmentOperator) {
    OGRPoint* point = new OGRPoint(2.0, 48.0);
    Geometry4D geom1(point);
    geom1.setT(2024.0);
    Geometry4D geom2;
    geom2 = geom1;
    EXPECT_DOUBLE_EQ(geom2.getT(), 2024.0);
}

// Teste le temps sur une ligne
TEST_F(Geometry4DTest, LineStringTimestamp) {
    OGRLineString* line = new OGRLineString();
    line->addPoint(0, 0);
    line->addPoint(1, 1);
    Geometry4D geom(line);
    geom.setT(2023.0);
    EXPECT_DOUBLE_EQ(geom.getT(), 2023.0);
}