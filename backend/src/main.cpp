#include <Application.hpp>
#include <iostream>

// // TEMP TESTING
// #include <iomanip>
// #include <string>
// #include <fstream>
// #include <cmath>
// #include <vector>
// #include <core/GeodeticTransformer.hpp>

// // === CONFIGURATION ===
// static const std::string DATA_PATH = "/app/backend/data/required/";

// // Codes couleurs pour la console
// const std::string GREEN = "\033[32m";
// const std::string RED   = "\033[31m";
// const std::string RESET = "\033[0m";

// // Fonction utilitaire pour vérifier l'existence du fichier
// std::string getModelPath(const std::string& filename) {
//     std::string fullPath = DATA_PATH + filename;
//     std::ifstream f(fullPath);
//     if (!f.good()) {
//         std::cerr << RED << "[ERREUR] Fichier introuvable : " << fullPath << RESET << std::endl;
//         return "";
//     }
//     return fullPath;
// }

// // Fonction utilitaire de comparaison (remplace EXPECT_NEAR)
// bool checkNear(const std::string& label, double actual, double expected, double epsilon) {
//     double diff = std::abs(actual - expected);
//     std::cout << std::left << std::setw(15) << label 
//               << " | Actuel: " << std::setw(12) << actual 
//               << " | Attendu: " << std::setw(12) << expected 
//               << " | Diff: " << diff;

//     if (diff <= epsilon) {
//         std::cout << GREEN << " [OK]" << RESET << std::endl;
//         return true;
//     } else {
//         std::cout << RED << " [FAIL]" << RESET << std::endl;
//         return false;
//     }
// }

// //////////////////////////////////////////////////////////////
// /// GEODETIC TESTS
// //////////////////////////////////////////////////////////////

// void testMayotteDefModelGeodetic(GeodeticTransformer& gt) {
//     std::cout << "\n==========================================" << std::endl;
//     std::cout << " TEST 8a : Defmodel JSON Mayotte Geodetic" << std::endl;
//     std::cout << "==========================================" << std::endl;

//     std::string model_path = getModelPath("fr_ign_RGM23_defmodel.json");
//     if (model_path.empty()) return;

//     try {
//         // Paramètres
//         double in_lon = 45.00;
//         double in_lat = -13.02;
//         double in_h   = 0.0;
//         double in_t   = 2018.0;

//         auto r = gt.applyDefModelGeodetic(in_lon, in_lat, in_h, in_t, model_path, false);

//         // Tolérances
//         double DEG_EPS = 1e-6; 
//         double H_EPS = 1e-3;

//         // Vérifications
//         checkNear("Longitude", r.x, 44.999998, DEG_EPS);
//         checkNear("Latitude",  r.y, -13.020001, DEG_EPS);
//         checkNear("Hauteur",   r.z, 0.0797, H_EPS);
        
//         if (r.t == 2018.0) std::cout << "Epoch           | " << r.t << GREEN << " [OK]" << RESET << std::endl;
//         else               std::cout << "Epoch           | " << r.t << RED << " [FAIL]" << RESET << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << RED << "Exception levée : " << e.what() << RESET << std::endl;
//     }
// }

// void testNKGGridDeformationGeodetic(GeodeticTransformer& gt) {
//     std::cout << "\n==========================================" << std::endl;
//     std::cout << " TEST 8b : NKG Grid deformation Geodetic" << std::endl;
//     std::cout << "==========================================" << std::endl;

//     std::string model_path = getModelPath("eur_nkg_nkgrf03vel_realigned.tif");
//     if (model_path.empty()) return;

//     try {
//         // Paramètres
//         double in_lon = 24.3953152240;
//         double in_lat = 60.2174694086;
//         double in_h   = 94.6218;
//         double in_t   = 2019.7000;
//         double t_target = 2000.0;

//         auto r = gt.applyGridDeformationGeodetic(in_lon, in_lat, in_h, in_t, model_path, t_target);

//         // Tolérances
//         double DEG_EPS = 1e-6; 
//         double H_EPS = 1e-3;

//         // Vérifications
//         checkNear("Longitude", r.x, 24.395315, DEG_EPS);
//         checkNear("Latitude",  r.y, 60.217469, DEG_EPS);
//         checkNear("Hauteur",   r.z, 94.6879, H_EPS);
        
//         if (r.t == 2019.7) std::cout << "Epoch           | " << r.t << GREEN << " [OK]" << RESET << std::endl;
//         else               std::cout << "Epoch           | " << r.t << RED << " [FAIL]" << RESET << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << RED << "Exception levée : " << e.what() << RESET << std::endl;
//     }
// }

// //////////////////////////////////////////////////////////////
// /// GEOCENTRIC TESTS
// //////////////////////////////////////////////////////////////

// void testMayotteDefModelGeocentric(GeodeticTransformer& gt) {
//     std::cout << "\n==========================================" << std::endl;
//     std::cout << " TEST 9a : Defmodel JSON Mayotte Geocentric" << std::endl;
//     std::cout << "==========================================" << std::endl;

//     std::string model_path = getModelPath("fr_ign_RGM23_defmodel.json");
//     if (model_path.empty()) return;

//     try {
//         // Paramètres
//         double in_X = 4394824.5976;
//         double in_Y = 4394824.5976;
//         double in_Z = -1427561.0242;
//         double in_t = 2018.0;

//         auto r = gt.applyDefModelGeocentric(in_X, in_Y, in_Z, in_t, model_path, false);

//         // Tolérances
//         double DEG_EPS = 2e-5; 
//         double H_EPS = 1e-3;

//         // Vérifications
//         checkNear("Longitude", r.x, 4394824.769114, DEG_EPS);
//         checkNear("Latitude",  r.y, 4394824.510683, DEG_EPS);
//         checkNear("Hauteur",   r.z, -1427561.118975, H_EPS);
        
//         if (r.t == 2018.0) std::cout << "Epoch           | " << r.t << GREEN << " [OK]" << RESET << std::endl;
//         else               std::cout << "Epoch           | " << r.t << RED << " [FAIL]" << RESET << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << RED << "Exception levée : " << e.what() << RESET << std::endl;
//     }
// }

// void testNKGGridDeformationGeocentric(GeodeticTransformer& gt) {
//     std::cout << "\n==========================================" << std::endl;
//     std::cout << " TEST 9b : NKG Grid deformation Geocentric" << std::endl;
//     std::cout << "==========================================" << std::endl;

//     std::string model_path = getModelPath("eur_nkg_nkgrf03vel_realigned.tif");
//     if (model_path.empty()) return;

//     try {
//         // Paramètres
//         double in_X =  2892571.15;
//         double in_Y =  1311843.30;
//         double in_Z =  5512633.99;
//         double in_t =  2019.7;
//         double t_target = 2000.0;

//         auto r = gt.applyGridDeformationGeocentric(in_X, in_Y, in_Z, in_t, model_path, t_target);

//         // Tolérances
//         double XYZ_EPS = 1e-6; 

//         // Vérifications
//         checkNear("X", r.x, 2892571.194434, XYZ_EPS);
//         checkNear("Y", r.y, 1311843.326880, XYZ_EPS);
//         checkNear("Z", r.z, 5512634.036652, XYZ_EPS);
        
//         if (r.t == 2019.7) std::cout << "Epoch           | " << r.t << GREEN << " [OK]" << RESET << std::endl;
//         else               std::cout << "Epoch           | " << r.t << RED << " [FAIL]" << RESET << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << RED << "Exception levée : " << e.what() << RESET << std::endl;
//     }

// }

// ///////////////////////////////////////////////////////////////
// /// PROJECTED TESTS
// ///////////////////////////////////////////////////////////////

// void testMayotteDefModelProjected(GeodeticTransformer& gt) {
//     std::cout << "\n==========================================" << std::endl;
//     std::cout << " TEST 10a : Defmodel JSON Mayotte Projected" << std::endl;
//     std::cout << "==========================================" << std::endl;

//     std::string model_path = getModelPath("fr_ign_RGM23_defmodel.json");
//     if (model_path.empty()) return;

//     try {
//         // Paramètres
//         double in_X = -3924135.51;
//         double in_Y = 16889560.57;
//         double in_Z = 0.0;
//         double in_t = 2018.0;

//         auto r = gt.applyDefModelProjected(in_X, in_Y, in_Z, in_t, model_path, false);

//         // Tolérances
//         double DEG_EPS = 1e-6; 
//         double H_EPS = 1e-3;

//         // Vérifications
//         checkNear("Longitude", r.x, -3924135.79, DEG_EPS);
//         checkNear("Latitude",  r.y, 16889560.46, DEG_EPS);
//         checkNear("Hauteur",   r.z, 0.08, H_EPS);
        
//         if (r.t == 2018.0) std::cout << "Epoch           | " << r.t << GREEN << " [OK]" << RESET << std::endl;
//         else               std::cout << "Epoch           | " << r.t << RED << " [FAIL]" << RESET << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << RED << "Exception levée : " << e.what() << RESET << std::endl;
//     }
// }

// void testNKGGridDeformationProjected(GeodeticTransformer& gt) {
//     std::cout << "\n==========================================" << std::endl;
//     std::cout << " TEST 10b : NKG Grid deformation Projected" << std::endl;
//     std::cout << "==========================================" << std::endl;

//     std::string model_path = getModelPath("eur_nkg_nkgrf03vel_realigned.tif");
//     if (model_path.empty()) return;

//     try {
//         // Paramètres
//         double in_X =  2892571.15;
//         double in_Y =  1311843.30;
//         double in_Z =  5512633.99;
//         double in_t =  2019.7;
//         double t_target = 2000.0;

//         auto r = gt.applyGridDeformationProjected(in_X, in_Y, in_Z, in_t, model_path, t_target);

//         // Tolérances
//         double XYZ_EPS = 1e-6; 

//         // Vérifications
//         checkNear("X", r.x, 2892571.194434, XYZ_EPS);
//         checkNear("Y", r.y, 1311843.326880, XYZ_EPS);
//         checkNear("Z", r.z, 5512634.036652, XYZ_EPS);
        
//         if (r.t == 2019.7) std::cout << "Epoch           | " << r.t << GREEN << " [OK]" << RESET << std::endl;
//         else               std::cout << "Epoch           | " << r.t << RED << " [FAIL]" << RESET << std::endl;

//     } catch (const std::exception& e) {
//         std::cerr << RED << "Exception levée : " << e.what() << RESET << std::endl;
//     }

// }

int main() {
    try {
        // // TEMP TESTING MANUALS
        // std::cout << std::fixed << std::setprecision(9);
    
        // // Instanciation du transformateur
        // GeodeticTransformer gt;

        // // Lancement des tests manuels
        // testMayotteDefModelGeodetic(gt);
        // testNKGGridDeformationGeodetic(gt);

        // testMayotteDefModelGeocentric(gt);
        // testNKGGridDeformationGeocentric(gt);

        // testMayotteDefModelProjected(gt);
        // testNKGGridDeformationProjected(gt);

        // std::cout << "\nFin des tests manuels." << std::endl;
        Application app;
        app.initialize();
        app.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
