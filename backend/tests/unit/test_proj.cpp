#include <gtest/gtest.h>
#include <core/geodetictransformer.hpp>

// === Chemin vers les données ===
static const std::string DATA_PATH = "/app/backend/data/required/";

// === Tolérance numérique acceptable ===
static constexpr double EPS = 0.0001;

// Fixture (optionnel)
class GeodeticTransformerTest : public ::testing::Test {
protected:
    GeodeticTransformer gt;
};

// ---------------------------------------------------------------------------
// TEST 1 : ITRF2020 -> ITRF2014
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, TransformITRF2020toITRF2014) {

    auto r = gt.transformAtEpoch(
        5.77338693, 43.97942110, 774.998, 2016.7474,
        "9989", "7912"
    );

    // Valeurs attendues (issues de l'image fournie)
    EXPECT_NEAR(r.x, 5.77339, EPS);
    EXPECT_NEAR(r.y, 43.9794, EPS);
    EXPECT_NEAR(r.z, 774.994, EPS);
}

// ---------------------------------------------------------------------------
// TEST 2 : Defmodel JSON Mayotte
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, MayotteDefModel) {

    auto r = gt.applyDefModel(
        45.00, -13.02, 0.0, 2018.0,
        DATA_PATH + "fr_ign_RGM23_defmodel.json",
        false
    );

    // Valeurs attendues
    EXPECT_NEAR(r.x, 44.0, EPS);
    EXPECT_NEAR(r.y, -13.02, EPS);
    EXPECT_NEAR(r.z, 0.0796542, EPS);   // valeur vue dans ton image
    EXPECT_NEAR(r.t, 2018.0, EPS);
}

// ---------------------------------------------------------------------------
// TEST 3 : NKG Grid deformation
// ---------------------------------------------------------------------------
TEST_F(GeodeticTransformerTest, NKGGridDeformation) {

    auto r = gt.applyGridDeformation(
        24.3953152240, 60.2174694086, 94.6218, 2019.7000,
        DATA_PATH + "eur_nkg_nkgrf03vel_realigned.tif",
        2000.0
    );

    // Valeurs attendues
    EXPECT_NEAR(r.x, 24.3953, EPS);
    EXPECT_NEAR(r.y, 60.2175, EPS);
    EXPECT_NEAR(r.z, 94.6879, EPS);
    EXPECT_NEAR(r.t, 2019.7, EPS);
}
