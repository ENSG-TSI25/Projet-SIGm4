#include "../include/ProjectCaracteristicsDisplay.h"

ProjectCarateristicsDisplay::ProjectCarateristicsDisplay(MainWindow* mw){

    nameLabel = new QLabel("Nom du projet: ");
    CRSLabel = new QLabel("CRS du projet: ");
    epoch0Label = new QLabel("Époque du projet: ");
    QVBoxLayout *newLayout = new QVBoxLayout(this);
    layout = newLayout;

    layout->addWidget(nameLabel);
    layout->addWidget(CRSLabel);
    layout->addWidget(epoch0Label);
    this->setLayout(layout);
};


ProjectCarateristicsDisplay::~ProjectCarateristicsDisplay(){

};

void ProjectCarateristicsDisplay::setProjectName(std::string newName){
    projectName = newName;
    nameLabel->setText("Nom du projet: " + QString::fromStdString(projectName));
}

void ProjectCarateristicsDisplay::setProjectEpoch0(double newEpoch0){
    projectEpoch0 = newEpoch0;
    epoch0Label->setText("Époque du projet: " + QString::number(projectEpoch0));
}

void ProjectCarateristicsDisplay::setProjectCRS(std::string newCRS){
    projectCRS = newCRS;
    CRSLabel->setText("CRS du projet: " + QString::fromStdString(projectCRS));
}
