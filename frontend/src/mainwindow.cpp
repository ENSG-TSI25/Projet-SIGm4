#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QComboBox>
#include <QString>
#include <QStringList>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileName(" ") 
{
    ui->setupUi(this);
    connect (ui->importBtn, &QPushButton::clicked, this, &MainWindow::listFiles);
    connect (ui->sourceCRSCombo, &QComboBox::currentTextChanged, this, &MainWindow::selectCRSsource);
    listDimension(); //dimension pour afficher le contenu de la combobox
    listSourceSys();
    listTargetSys();
    carte = new Carte(ui->carte); // ui->frameCarte = ton QFrame dans Qt Designer

    
    connect (ui->targetCRSCombo, &QComboBox::currentTextChanged, this, &MainWindow::selectCRSdest);

    connect (ui->epochEdit, &QLineEdit::textEdited, this, &MainWindow::getDate);
    connect (ui->transformBtn, &QPushButton::clicked, this, &MainWindow::transform);

    connect (ui->addToMapBtn, &QPushButton::clicked, this, &MainWindow::addFileToWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listFiles(){
    fileName = QFileDialog::getOpenFileName(this, tr("Open window"), "$PWD", tr("Files (*.gpkg)"));
}


void MainWindow::listDimension(){

    ui->dimensionCombo->clear();
    ui->dimensionCombo->addItem("2D");
    ui->dimensionCombo->addItem("4D");

}

void MainWindow::listSourceSys(){

      ui->sourceCRSCombo->clear();

        QStringList items = {
            "ITRF2020 (9990)",
            "ITRF2014 (9000)",
            "ITRF2008 (8999)",
            "ITRF2005 (8998)",
            "ITRF2000 (8987)",
            "ETRF2020 (10571)",
            "ETRF2014 (9069)",
            "ETRF2005 (9068)",
            "ETRF2000 (9067)",
            "RGF93v2b (9784)",
            "RGM23 (10673)"
        };

        ui->sourceCRSCombo->addItems(items);
    }
void MainWindow::listTargetSys(){

      ui->targetCRSCombo->clear();

        QStringList items = {
            "ITRF2020 (9990)",
            "ITRF2014 (9000)",
            "ITRF2008 (8999)",
            "ITRF2005 (8998)",
            "ITRF2000 (8987)",
            "ETRF2020 (10571)",
            "ETRF2014 (9069)",
            "ETRF2005 (9068)",
            "ETRF2000 (9067)",
            "RGF93v2b (9784)",
            "RGM23 (10673)"
        };

        ui->targetCRSCombo->addItems(items);
    }

std::string MainWindow::selectCRSsource() {
    QString text = ui -> sourceCRSCombo->currentText();
    std::string crs = text.toStdString();
    int del = 0;
    for (int i=0; i<=crs.length();i++) {
        if (crs[i] == '(') {
            del = i+1;
        }
    }
    crs.erase(0,del);
    crs.erase(crs.length()-1);
    return crs;
}

std::string MainWindow::selectCRSdest() {
    QString text = ui -> targetCRSCombo->currentText();
    std::string crs = text.toStdString();
    int del = 0;
    for (int i=0; i<=crs.length();i++) {
        if (crs[i] == '(') {
            del = i+1;
        }
    }
    crs.erase(0,del);
    crs.erase(crs.length()-1);
    return crs;
}

double MainWindow::getDate() {
    QString date = ui -> epochEdit -> text();
    return date.toDouble();
}

std::tuple<std::string, std::string, double> MainWindow::transform() {
    std::tuple<std::string, std::string, double> final = {selectCRSsource(),selectCRSdest(), getDate()};
    std::cout<<std::get<0>(final);
    std::cout<<std::get<1>(final);
    std::cout<<std::get<2>(final);
    return final;
}

void MainWindow::addFileToWidget() {
    if (!fileName.isEmpty()) {
        QStringList filenameChar = fileName.split(u'/');
        ui -> layersList -> addItem(filenameChar.last());
        fileName = "";
    }
}

