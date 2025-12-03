#include "projet.h"

std::string Projet::getProjectCrs(){
    return projectCRS;
}

double Projet::getProjectEpoch(){
    return projectEpoch;
}

void Projet::setProjectCrs(std::string CRS){
    this->projectCRS = CRS;
}

void Projet::setProjectEpoch(double epoch){
    this->projectEpoch = epoch;
}