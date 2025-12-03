#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect (ui->importBtn, &QPushButton::clicked, this, &MainWindow::listFiles);
    listDimension(); //dimension pour a fficher le contenu de la combobox
    listSourceSys();
    listTargetSys();
    carte = new Carte(ui->carte); // ui->frameCarte = ton QFrame dans Qt Designer
    connect (ui->btnNew, &QPushButton::clicked, this, &MainWindow::clickNewProject);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listFiles(){
       QString fileName = QFileDialog::getOpenFileName(this, tr("Open window"), "$PWD", tr("Files (*.gpkg)"));}


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
            "RGM23 (10673)",
            "autres"
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
            "RGM23 (10673)",
            "autres"
        };

        ui->targetCRSCombo->addItems(items);
    }

void MainWindow::clickNewProject(){
        //On crée la boîte de dialogue
    QDialog chosingCRSDialog;
    chosingCRSDialog.setWindowTitle("Choix du CRS");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);
    QLabel *dialogText = new QLabel("Choisissez un CRS et une époque pour votre projet", &chosingCRSDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);
    QLineEdit *crsTextZone = new QLineEdit(&chosingCRSDialog);
    crsTextZone->setPlaceholderText("Entrez le code EPSG du CRS");
    QLineEdit *epochTextZone = new QLineEdit(&chosingCRSDialog);
    epochTextZone->setPlaceholderText("Entrez l'époque");

    //On crée un validateur pour vérifier que l'utilisateur ne rentre bien que des doubles
    QDoubleValidator *doubleValidator = new QDoubleValidator();
    //La range et les décimales
    doubleValidator->setRange(0, 2030, 3);
    //On intègre le validateur à la zone de texte
    epochTextZone->setValidator(doubleValidator);


    //on ajoute les widgets
    layout->addWidget(dialogText);
    layout->addWidget(crsTextZone);
    layout->addWidget(epochTextZone);
    layout->addWidget(acceptationButton);

    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);

    chosingCRSDialog.exec();
}
