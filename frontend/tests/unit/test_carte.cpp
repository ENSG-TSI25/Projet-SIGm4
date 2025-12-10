#include <gtest/gtest.h>
#include <cmath>
// // #include <QApplication>
// // #include <qgsapplication.h>
// // #include <qgscoordinatereferencesystem.h>
// // #include "Carte.h"

// // ============================================================================
// // FIXTURE : Configuration commune à tous les tests
// // ============================================================================
// class CarteTest : public ::testing::Test {
// protected:
//     // Appelé AVANT chaque test
//     void SetUp() override {
//         // Initialisation de QGIS (nécessaire pour QgsPointXY)
//         // À faire UNE SEULE FOIS dans main(), pas dans SetUp()
//     }

//     // Appelé APRÈS chaque test
//     void TearDown() override {
//         // Nettoyage si nécessaire
//     }

//     // Helper : compare deux doubles avec une tolérance
//     bool areClose(double a, double b, double tolerance = 1e-6) {
//         return std::abs(a - b) < tolerance;
//     }
// };

// // ============================================================================
// // TESTS DE LA FONCTION wgs84ToMercator
// // ============================================================================

// // Test 1 : Paris (cas nominal)
// TEST_F(CarteTest, Wgs84ToMercator_Paris) {
//     // Arrange : Coordonnées de Paris
//     double lon = 2.3522;  // longitude
//     double lat = 48.8566; // latitude
    
//     // Act : Conversion
//     QgsPointXY result = Carte::wgs84ToMercator(lon, lat);
    
//     // Assert : Valeurs attendues (calculées avec une source de référence)
//     double expectedX = 261945.0;  // mètres en Web Mercator
//     double expectedY = 6250930.0; // mètres en Web Mercator
    
//     EXPECT_TRUE(areClose(result.x(), expectedX, 1.0)) 
//         << "X attendu: " << expectedX << ", obtenu: " << result.x();
//     EXPECT_TRUE(areClose(result.y(), expectedY, 1.0))
//         << "Y attendu: " << expectedY << ", obtenu: " << result.y();
// }

// // Test 2 : Équateur (lat = 0)
// TEST_F(CarteTest, Wgs84ToMercator_Equateur) {
//     double lon = 0.0;
//     double lat = 0.0;
    
//     QgsPointXY result = Carte::wgs84ToMercator(lon, lat);
    
//     // À l'équateur et au méridien de Greenwich
//     EXPECT_NEAR(result.x(), 0.0, 1e-6);
//     EXPECT_NEAR(result.y(), 0.0, 1e-6);
// }

// // Test 3 : Longitude extrême (antéméridien)
// TEST_F(CarteTest, Wgs84ToMercator_Antemeridien) {
//     double lon = 180.0;
//     double lat = 0.0;
    
//     QgsPointXY result = Carte::wgs84ToMercator(lon, lat);
    
//     // 180° = limite est de la projection
//     double expectedX = 20037508.34;
//     EXPECT_NEAR(result.x(), expectedX, 1.0);
//     EXPECT_NEAR(result.y(), 0.0, 1e-6);
// }

// // Test 4 : Longitude négative (ouest)
// TEST_F(CarteTest, Wgs84ToMercator_LongitudeOuest) {
//     double lon = -180.0;
//     double lat = 0.0;
    
//     QgsPointXY result = Carte::wgs84ToMercator(lon, lat);
    
//     // -180° = limite ouest
//     double expectedX = -20037508.34;
//     EXPECT_NEAR(result.x(), expectedX, 1.0);
// }

// // Test 5 : Latitude proche du pôle Nord
// TEST_F(CarteTest, Wgs84ToMercator_PoleNord) {
//     double lon = 0.0;
//     double lat = 85.0; // Web Mercator valide jusqu'à ~85°
    
//     QgsPointXY result = Carte::wgs84ToMercator(lon, lat);
    
//     // Y doit être très grand mais fini
//     EXPECT_GT(result.y(), 19000000.0);
//     EXPECT_LT(result.y(), 21000000.0);
// }

// // Test 6 : Symétrie Nord/Sud
// TEST_F(CarteTest, Wgs84ToMercator_Symetrie) {
//     double lon = 5.0;
//     double latNord = 45.0;
//     double latSud = -45.0;
    
//     QgsPointXY nord = Carte::wgs84ToMercator(lon, latNord);
//     QgsPointXY sud = Carte::wgs84ToMercator(lon, latSud);
    
//     // X doit être identique
//     EXPECT_NEAR(nord.x(), sud.x(), 1e-6);
    
//     // Y doit être opposé (symétrie)
//     EXPECT_NEAR(nord.y(), -sud.y(), 1.0);
// }

// // Test 7 : Lyon (cas réel supplémentaire)
// TEST_F(CarteTest, Wgs84ToMercator_Lyon) {
//     double lon = 4.8357;
//     double lat = 45.7640;
    
//     QgsPointXY result = Carte::wgs84ToMercator(lon, lat);
    
//     // Vérification que les valeurs sont dans un ordre de grandeur cohérent
//     EXPECT_GT(result.x(), 500000.0);
//     EXPECT_LT(result.x(), 600000.0);
//     EXPECT_GT(result.y(), 5700000.0);
//     EXPECT_LT(result.y(), 5800000.0);
// }

// // ============================================================================
// // TESTS SUPPLÉMENTAIRES (si tu refactorises toggleBaseLayer)
// // ============================================================================

// // Si tu extrais la logique dans une fonction testable :
// /*
// TEST_F(CarteTest, ToggleLogic_OsmToSat) {
//     bool osmVisible = true;
//     bool result = !osmVisible;
    
//     EXPECT_FALSE(result);
// }

// TEST_F(CarteTest, ToggleLogic_SatToOsm) {
//     bool osmVisible = false;
//     bool result = !osmVisible;
    
//     EXPECT_TRUE(result);
// }
// */

// // ============================================================================
// // TESTS PARAMÉTRÉS (pour tester plusieurs cas similaires)
// // ============================================================================

// // Structure pour les cas de test
// struct CoordTestCase {
//     double lon;
//     double lat;
//     double expectedX;
//     double expectedY;
//     std::string description;
// };

// class CarteParametrizedTest : public ::testing::TestWithParam<CoordTestCase> {};

// TEST_P(CarteParametrizedTest, MultipleCoordinates) {
//     CoordTestCase testCase = GetParam();
    
//     QgsPointXY result = Carte::wgs84ToMercator(testCase.lon, testCase.lat);
    
//     EXPECT_NEAR(result.x(), testCase.expectedX, 1000.0) << testCase.description;
//     EXPECT_NEAR(result.y(), testCase.expectedY, 1000.0) << testCase.description;
// }

// // Définition des cas de test
// INSTANTIATE_TEST_SUITE_P(
//     VillesFrancaises,
//     CarteParametrizedTest,
//     ::testing::Values(
//         CoordTestCase{2.3522, 48.8566, 261945, 6250930, "Paris"},
//         CoordTestCase{4.8357, 45.7640, 538322, 5754687, "Lyon"},
//         CoordTestCase{5.3698, 43.2965, 597635, 5381854, "Marseille"},
//         CoordTestCase{-1.5534, 47.2184, -172925, 5990716, "Nantes"}
//     )
// );

// // ============================================================================
// // MAIN : Point d'entrée des tests
// // ============================================================================
// int main(int argc, char **argv) {
//     // Initialisation de Qt (nécessaire pour QgsPointXY)
//     QApplication app(argc, argv);
    
//     // Initialisation de QGIS
//     QgsApplication::init();
    
//     // Initialisation de Google Test
//     ::testing::InitGoogleTest(&argc, argv);
    
//     // Exécution de tous les tests
//     int result = RUN_ALL_TESTS();
    
//     // Nettoyage QGIS
//     QgsApplication::exitQgis();
    
//     return result;
// }