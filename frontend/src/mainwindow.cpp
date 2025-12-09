#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDoubleValidator>
#include <QDialog>
#include <QDialogButtonBox>

#include <QString>
#include <QStringList>
#include <QListWidget>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) 
    , fileName(" ") 
    
{
    ui->setupUi(this);
    connect (ui->importBtn, &QPushButton::clicked, this, &MainWindow::listFiles);
    //For displaying the CRSs list on the source and target Comboboxes
    setCrsList(ui->sourceCRSCombo);
    setCrsList(ui->targetCRSCombo);
    connect (ui->sourceCRSCombo, &QComboBox::currentTextChanged, this, &MainWindow::selectCRSsource);
    connect (ui->targetCRSCombo, &QComboBox::currentTextChanged, this, &MainWindow::selectCRSdest);
    listDimension(); //dimension pour afficher le contenu de la combobox
    carte = new Carte(ui->carte);
    connect(carte->getCanvas(),&QgsMapCanvas::scaleChanged, this,&MainWindow::updateScaleLabel);    connect (ui->btnZoomPlus, &QPushButton::clicked, this, &MainWindow::zoomIn_button);
    connect (ui->btnZoomMinus, &QPushButton::clicked, this, &MainWindow::zoomOut_button);
      
    

    connect (ui->epochEdit, &QLineEdit::textEdited, this, &MainWindow::getDate);
    connect (ui->transformBtn, &QPushButton::clicked, this, &MainWindow::transform);

    connect (ui->addToMapBtn, &QPushButton::clicked, this, &MainWindow::addFileToWidget);

    //When the "Nouveau" button is clicked, open a new window for choosing the CRS and the eopch
    connect (ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);

    //Dialog management
    dialog = new Dialog();
    connect (ui->layersList, &QListWidget::itemActivated, this, &MainWindow::openDialog);
    Ui::Dialog *dig = dialog -> getUI();
    connect (dig->buttonDuplicate, &QPushButton::clicked, this, &MainWindow::duplicateLayer);
    connect (dig->buttonRename, &QPushButton::clicked, this, &MainWindow::renameLayer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateScaleLabel(int scaleValue)
{
    ui->scale->setText(QString("Échelle : 1:%1").arg(QString::number((std::round(scaleValue/100)*100), 'f', 0)));

}

void MainWindow::zoomIn_button()
{
    carte->getCanvas()->zoomIn();
}


void MainWindow::zoomOut_button()
{
    carte->getCanvas()->zoomOut();
}


void MainWindow::listFiles(){
    fileName = QFileDialog::getOpenFileName(this, tr("Open window"), "$PWD", tr("Files (*.gpkg)"));
}


void MainWindow::listDimension(){

    ui->dimensionCombo->clear();
    ui->dimensionCombo->addItem("2D");
    ui->dimensionCombo->addItem("4D");

}

std::string MainWindow::selectCRSsource() {
    QString text = ui->sourceCRSCombo->currentText();
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
        QString layerName = filenameChar.last();
        QListWidgetItem *item = new QListWidgetItem(layerName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);

        ui->layersList->addItem(item);

        //VectorLayer vectLayer = chargerVecteur(fileName);
        //QgsMapCanvas* canva = carte->getCanvas();
        //canva->setLayers({vectLayer});

        fileName = "";
    }
}

//Duplicate the layer when it's clicked
void MainWindow::duplicateLayer() {
    QString name = dialog-> nameLayer();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    int currentIndex = ui -> layersList -> row(ui -> layersList -> currentItem());
    ui -> layersList -> insertItem(currentIndex, item);
}

//Rename layer selected
void MainWindow::renameLayer() {
    duplicateLayer();
    int currentIndex = ui -> layersList -> row(ui -> layersList -> currentItem());
    ui -> layersList -> takeItem(currentIndex);
}

//Open dialog when the layer is clicked
void MainWindow::openDialog() {
    dialog -> show();
}

//The function to set the CRS and the epoch of a new project when clicking on "Nouveau"
void MainWindow::setNewProject(){
    //On crée la boîte de dialogue
    QDialog chosingCRSDialog;
    chosingCRSDialog.setWindowTitle("Choix du CRS");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);
    QLabel *dialogText = new QLabel("Choisissez un CRS et une époque pour votre projet", &chosingCRSDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);
    QComboBox *crsList = new QComboBox(&chosingCRSDialog);
    crsList->setPlaceholderText("Entrez le code EPSG du CRS");
    setCrsList(crsList);
    QLineEdit *epochTextZone = new QLineEdit(&chosingCRSDialog);
    epochTextZone->setPlaceholderText("Entrez l'époque");

    //On crée un validateur pour vérifier que l'utilisateur ne rentre bien que des doubles
    QDoubleValidator *doubleValidator = new QDoubleValidator(&chosingCRSDialog);
    //La range et les décimales
    doubleValidator->setRange(0, 2030, 3);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    //On intègre le validateur à la zone de texte
    epochTextZone->setValidator(doubleValidator);


    //on ajoute les widgets
    layout->addWidget(dialogText);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(acceptationButton);

    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);

    chosingCRSDialog.exec();
}

//Function to set the targetted comboBox to show the list of CRS accepted by the project
void MainWindow::setCrsList(QComboBox *comboBox){
        comboBox->clear();
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
        comboBox->addItems(items);
}
