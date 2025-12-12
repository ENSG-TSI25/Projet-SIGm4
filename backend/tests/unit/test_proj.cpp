#include <gtest/gtest.h>
#include <core/GeodeticTransformer.hpp>

// === Path to data ===
static const std::string DATA_PATH = "/app/backend/data/required/";

// Fixture
class GeodeticTransformerTest : public ::testing::Test {
protected:
    GeodeticTransformer gt;
};

// ---------------------------------------------------------------------------
// TEST 1 : ITRF2020 -> ITRF2014 (Geodetic to Geodetic)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, TransformITRF2020toITRF2014) {
    // Source : Lon, Lat, H, Epoch
    auto r = gt.transformAtEpoch(
        5.77338693, 43.97942110, 774.998, 2016.7474,
        "9989", "7912"
    );

    double DEG_EPS = 1e-4; 
    double H_EPS = 1e-3;

    // r.x = Longitude, r.y = Latitude
    EXPECT_NEAR(r.x, 5.77339, DEG_EPS);
    EXPECT_NEAR(r.y, 43.9794, DEG_EPS);
    EXPECT_NEAR(r.z, 774.994, H_EPS);
}

// ---------------------------------------------------------------------------
// TEST 2 : Defmodel JSON Mayotte
// ---------------------------------------------------------------------------
// TEST_F(GeodeticTransformerTest, MayotteDefModel) {

//     auto r = gt.applyDefModel(
//         45.00, -13.02, 0.0, 2018.0,
//         DATA_PATH + "fr_ign_RGM23_defmodel.json",
//         false
//     );

//     // Tolerances : 
//     double DEG_EPS = 1e-4; 
//     double H_EPS = 1e-3;
//
//     // Valeurs attendues
//     EXPECT_NEAR(r.x, 44.0, DEG_EPS);
//     EXPECT_NEAR(r.y, -13.02, DEG_EPS);
//     EXPECT_NEAR(r.z, 0.0796542, H_EPS); 
//     EXPECT_EQ(r.t, 2018.0);
// }

// // ---------------------------------------------------------------------------
// // TEST 3 : NKG Grid deformation
// // ---------------------------------------------------------------------------
// TEST_F(GeodeticTransformerTest, NKGGridDeformation) {

//     auto r = gt.applyGridDeformation(
//         24.3953152240, 60.2174694086, 94.6218, 2019.7000,
//         DATA_PATH + "eur_nkg_nkgrf03vel_realigned.tif",
//         2000.0
//     );
//
//     // Tolerances : 
//     double DEG_EPS = 1e-4; 
//     double H_EPS = 1e-3;
//
//     // Valeurs attendues
//     EXPECT_NEAR(r.x, 24.3953, DEG_EPS);
//     EXPECT_NEAR(r.y, 60.2175, DEG_EPS);
//     EXPECT_NEAR(r.z, 94.6879, H_EPS);
//     EXPECT_EQ(r.t, 2019.7);
// }

// ---------------------------------------------------------------------------
// TEST 4 : Geocentric/Cartesian (XYZ) to Geodetic (Lon, Lat, H)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeocentricToGeodetic) {

    // Données d'entrée (XYZ ECEF)
    double input_x = 4201713.435;
    double input_y = 168435.698;
    double input_z = 4779483.504;
    double input_t = 2024.5;

    auto r = gt.geocentricToGeodetic(input_x, input_y, input_z, input_t);

    double DEG_EPS = 1e-4; 
    double H_EPS = 1e-3;
    
    EXPECT_NEAR(r.x, 2.2956087354, DEG_EPS);

    EXPECT_NEAR(r.y, 48.8487915744, DEG_EPS);

    EXPECT_NEAR(r.z, -35.1480, H_EPS);

    EXPECT_EQ(r.t, 2024.5);
}

// ---------------------------------------------------------------------------
// TEST 5 : Geodetic (Lon, Lat, H) to Geocentric/Cartesian (XYZ)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeodeticToGeocentric) {

    // Source : WGS84 (EPSG:4326)
    double input_lon = 2.2956087354;
    double input_lat = 48.8487915744;
    double input_h   = -35.1480;
    double input_t   = 2024.5;

    // Appel : (lon, lat, h, t)
    auto r = gt.geodeticToGeocentric(input_lon, input_lat, input_h, input_t);

    double XYZ_EPS = 1e-3; // 1mm

    // r.x = X, r.y = Y, r.z = Z
    EXPECT_NEAR(r.x, 4201713.435, XYZ_EPS);
    EXPECT_NEAR(r.y, 168435.698, XYZ_EPS);
    EXPECT_NEAR(r.z, 4779483.504, XYZ_EPS);

    EXPECT_EQ(r.t, 2024.5);
}


// ---------------------------------------------------------------------------
// TEST 6 : Projected (Lambert-93) to Geodetic (Lon, Lat, H)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, ProjectedToGeodetic) {

    // Source : Lambert-93 (EPSG:2154)
    double input_x = 648240.5;   // Easting
    double input_y = 6862271.9;  // Northing
    double input_z = 100.0;
    double input_t = 2024.5;

    // Destination : WGS84 (EPSG:4326)
    auto r = gt.projectedToGeodetic(input_x, input_y, input_z, input_t, "EPSG:2154", "EPSG:4326");

    double DEG_EPS = 1e-4;
    double H_EPS = 1e-3;
    
    EXPECT_NEAR(r.x, 2.29454356, DEG_EPS);

    EXPECT_NEAR(r.y, 48.85840222, DEG_EPS);

    EXPECT_NEAR(r.z, 100.0, H_EPS); 
    EXPECT_EQ(r.t, 2024.5);
}

// ---------------------------------------------------------------------------
// TEST 7 : Geodetic (Lon, Lat, H) to Projected (Lambert-93)
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, GeodeticToProjected) {

    // Source : WGS84 (EPSG:4326)
    double input_lon = 2.29454356;
    double input_lat = 48.85840222;
    double input_z   = 100.0;
    double input_t   = 2024.5;

    auto r = gt.geodeticToProjected(input_lon, input_lat, input_z, input_t, "EPSG:4326", "EPSG:2154");

    double PROJ_EPS = 1e-1;
    double H_EPS = 1e-3;

    // r.x = Easting (X), r.y = Northing (Y)
    EXPECT_NEAR(r.x, 648240.5, PROJ_EPS);
    EXPECT_NEAR(r.y, 6862271.9, PROJ_EPS);

    EXPECT_NEAR(r.z, 100.0, H_EPS); 
    EXPECT_EQ(r.t, 2024.5);
}