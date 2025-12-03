#include "projet.h"

std::string Projet::getProjectCrs(){
    return projectCRS;
}

double Projet::getProjectEpoch(){
    return projectEpoch;
}

void Projet::setProjectCrs(std::string CRS){
    //cette fonction devra aussi changer le crs de projection et celui de toutes les couches du projet
    this->projectCRS = CRS;
}

void Projet::setProjectEpoch(double epoch){
    //cette fonction devra aussi changer le crs de projection et celui de toutes les couches du projet
    this->projectEpoch = epoch;
}