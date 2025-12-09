#include <proj.h>
#include "core/geodetictransformer.hpp"
#include "core/Geometry4D.hpp"
#include <iostream>
#include <cstdlib>

int proj_test() {
    
    std::cout<<"\nDébut des test de proj : "<<std::endl;

    std::string path = "/app/backend/data/required/";

    GeodeticTransformer gt;

    // === 1. ITRF2020 -> ITRF2014 ===
    auto r1 = gt.transformAtEpoch(
        5.77338693, 43.97942110, 774.998, 2016.7474,
        "9989", "7912");
    std::cout << "ITRF2020 -> ITRF2014 : "
            << r1.x << " " << r1.y << " " << r1.z << "\n";

    // // === 2. Mayotte modèle JSON ===
    auto r2 = gt.applyDefModel(
        45.00, -13.02, 0.0, 2018.0,
        path + "fr_ign_RGM23_defmodel.json",
        false);
    std::cout << "Mayotte defmodel -> "
            << r2.x << " " << r2.y << " " << r2.z << " " << r2.t <<"\n";

    // === 3. Scandinavie + grille ===
    auto r3 = gt.applyGridDeformation(
        24.3953152240, 60.2174694086, 94.6218, 2019.7000,
        path + "eur_nkg_nkgrf03vel_realigned.tif",
        2000.0);
    std::cout << "NKG grid deformation: "
            << r3.x << " " << r3.y << " " << r3.z << " " <<r3.t<<"\n";

    // Geometry4D geom;
    // std::cout << geom.getGeometry() << "\n";
            
    std::cout<<"Fin des tests proj"<<std::endl;

    return 0;
}