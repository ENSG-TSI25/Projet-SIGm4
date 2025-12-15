#include <gtest/gtest.h>
#include <core/GeodeticTransformer.hpp>

// Fixture
class GeodeticTransformerTest : public ::testing::Test {
protected:
    GeodeticTransformer gt;
};

// ---------------------------------------------------------------------------
// TEST 1 : ITRF2020 -> ITRF2014 (Geodetic to Geodetic)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, TransformGeodeticAtEpoch) {

    // Source : Lon, Lat, H, Epoch
    auto r = gt.transformGeodeticAtEpoch(
        5.77338693, 43.97942110, 774.998, 2016.7474,
        "EPSG:9989", "EPSG:7912"
    );

    double DEG_EPS = 1e-6; 
    double H_EPS = 2e-3;

    // r.x = Longitude, r.y = Latitude
    EXPECT_NEAR(r.x, 5.773387, DEG_EPS);
    EXPECT_NEAR(r.y, 43.979421, DEG_EPS);
    EXPECT_NEAR(r.z, 774.994, H_EPS);
    EXPECT_EQ(r.t, 2016.7474);
}

// ---------------------------------------------------------------------------
// TEST 2 : Lambert-93 -> WebMercator (Projected to Projected)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, TransformLinearAtEpoch) {

    auto r = gt.transformLinearAtEpoch(
        648240.5, 6862271.9, 100.0, 2024.5,
        "EPSG:2154", "EPSG:3857"
    );

    double XY_EPS = 1e-6;
    double H_EPS  = 1e-3;

    EXPECT_NEAR(r.x, 255427.421190, XY_EPS);
    EXPECT_NEAR(r.y, 6250869.277344, XY_EPS);
    EXPECT_NEAR(r.z, 100.000, H_EPS);
    EXPECT_EQ(r.t, 2024.5);
}

// ---------------------------------------------------------------------------
// TEST 3 : Geodetic -> Geocentric
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeodeticToGeocentric) {

    auto r = gt.geodeticToGeocentric(
        2.29453775, 48.85806174, -76.07810299, 2020.0,
        "EPSG:4979", "EPSG:4978"
    );

    double XYZ_EPS = 1e-6;

    EXPECT_NEAR(r.x, 4200913.999773, XYZ_EPS);
    EXPECT_NEAR(r.y, 168325.000191,  XYZ_EPS);
    EXPECT_NEAR(r.z, 4780131.000192, XYZ_EPS);
    EXPECT_EQ(r.t, 2020.0);
}

// ---------------------------------------------------------------------------
// TEST 4 : Geocentric -> Geodetic
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeocentricToGeodetic) {

    auto r = gt.geocentricToGeodetic(
        4200914.0, 168325.0, 4780131.0, 2020.0,
        "EPSG:4978", "EPSG:4979"
    );

    double DEG_EPS = 1e-6;
    double H_EPS   = 1e-3;

    EXPECT_NEAR(r.x, 2.294538, DEG_EPS);
    EXPECT_NEAR(r.y, 48.858062, DEG_EPS);
    EXPECT_NEAR(r.z, -76.078, H_EPS);
    EXPECT_EQ(r.t, 2020.0);
}

// ---------------------------------------------------------------------------
// TEST 5 : Projected -> Geodetic (Lambert-93 -> WGS84)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, ProjectedToGeodetic) {

    auto r = gt.projectedToGeodetic(
        648240.5, 6862271.9, 100.0, 2022.0,
        "EPSG:2154", "EPSG:4979"
    );

    double DEG_EPS = 1e-6;
    double H_EPS   = 1e-3;

    EXPECT_NEAR(r.x, 2.294544, DEG_EPS);
    EXPECT_NEAR(r.y, 48.858402, DEG_EPS);
    EXPECT_NEAR(r.z, 100.0, H_EPS);
    EXPECT_EQ(r.t, 2022.0);
}

// ---------------------------------------------------------------------------
// TEST 6 : Geodetic -> Projected (WGS84 -> Lambert-93)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeodeticToProjected) {

    auto r = gt.geodeticToProjected(
        2.2945, 48.8584, 100.0, 2022.0,
        "EPSG:4979", "EPSG:2154"
    );

    double XY_EPS = 1e-6;
    double H_EPS  = 1e-3;

    EXPECT_NEAR(r.x, 648237.301549, XY_EPS);
    EXPECT_NEAR(r.y, 6862271.681554, XY_EPS);
    EXPECT_NEAR(r.z, 100.0, H_EPS);
    EXPECT_EQ(r.t, 2022.0);
}

// ---------------------------------------------------------------------------
// TEST 7 : Projected -> Geocentric
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, ProjectedToGeocentric) {

    auto r = gt.projectedToGeocentric(
        648240.5, 6862271.9, 100.0, 2022.0,
        "EPSG:2154", "EPSG:4978"
    );

    double XYZ_EPS = 1e-6;

    EXPECT_NEAR(r.x, 4201001.243990, XYZ_EPS);
    EXPECT_NEAR(r.y, 168328.922953,  XYZ_EPS);
    EXPECT_NEAR(r.z, 4780288.513431, XYZ_EPS);
    EXPECT_EQ(r.t, 2022.0);
}

// ---------------------------------------------------------------------------
// TEST 8 : Geocentric -> Projected
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeocentricToProjected) {

    auto r = gt.geocentricToProjected(
        4200914.0, 168325.0, 4780131.0, 2022.0,
        "EPSG:4978", "EPSG:2154"
    );

    double XY_EPS = 1e-6;
    double H_EPS  = 1e-3;

    EXPECT_NEAR(r.x, 648239.734964, XY_EPS);
    EXPECT_NEAR(r.y, 6862234.045139, XY_EPS);
    EXPECT_NEAR(r.z, -76.078, H_EPS);
    EXPECT_EQ(r.t, 2022.0);
}

// // ---------------------------------------------------------------------------
// // TEST 9 : Defmodel JSON Mayotte
// // ---------------------------------------------------------------------------
// TEST_F(GeodeticTransformerTest, MayotteDefModel) {
//     std::string model_path = getModelPath("fr_ign_RGM23_defmodel.json");

//     auto r = gt.applyDefModelGeodetic(
//         45.00, -13.02, 0.0, 2018.0,
//         model_path,
//         false
//     );

//     // Tolerances : 
//     double DEG_EPS = 1e-4; 
//     double H_EPS = 1e-3;

//     // Valeurs attendues
//     EXPECT_NEAR(r.x, 45.0, DEG_EPS);
//     EXPECT_NEAR(r.y, -13.02, DEG_EPS);
//     EXPECT_NEAR(r.z, 0.0796542, H_EPS); 
//     EXPECT_EQ(r.t, 2018.0);
// }

// // ---------------------------------------------------------------------------
// // TEST 10 : NKG Grid deformation
// // ---------------------------------------------------------------------------
// TEST_F(GeodeticTransformerTest, NKGGridDeformation) {
//     std::string model_path = getModelPath("eur_nkg_nkgrf03vel_realigned.tif");

//     auto r = gt.applyGridDeformationGeodetic(
//         24.3953152240, 60.2174694086, 94.6218, 2019.7000,
//         model_path,
//         2000.0
//     );

//     // Tolerances : 
//     double DEG_EPS = 1e-4; 
//     double H_EPS = 1e-3;

//     // Valeurs attendues
//     EXPECT_NEAR(r.x, 24.3953, DEG_EPS);
//     EXPECT_NEAR(r.y, 60.2175, DEG_EPS);
//     EXPECT_NEAR(r.z, 94.6879, H_EPS);
//     EXPECT_EQ(r.t, 2019.7);
// }

