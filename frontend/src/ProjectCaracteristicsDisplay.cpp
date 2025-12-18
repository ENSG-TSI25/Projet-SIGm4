#include "../include/ProjectCaracteristicsDisplay.h"
#include "../include/mainwindow.h" 

ProjectCarateristicsDisplay::ProjectCarateristicsDisplay(MainWindow* mw):
    mw(mw)
    {
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

void ProjectCarateristicsDisplay::updateDisplayName(){
    nameLabel->setText("Nom du projet: " + QString::fromStdString(mw->getCurrentProject()->getName()));
}

void ProjectCarateristicsDisplay::updateDisplayEpoch0(){
    qDebug() << QString::fromStdString(mw->getCurrentProject()->getName());
    epoch0Label->setText("Époque du projet: " + QString::number(mw->getCurrentProject()->getEpoch0()));
}

void ProjectCarateristicsDisplay::updateDisplayCRS(){
    CRSLabel->setText("CRS du projet: " + QString::fromStdString(mw->getCurrentProject()->getCrs()));
}
