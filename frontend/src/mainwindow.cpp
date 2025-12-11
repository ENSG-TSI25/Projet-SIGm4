#include "../include/mainwindow.h"
#include "../include/LayerManager.h"
#include "../include/TransformCRS.h"
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
#include <QCalendarWidget>
#include <QDialogButtonBox>

#include <QString>
#include <QStringList>
#include <QListWidget>
#include <iostream>
#include <QDate>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) 
{
    ui->setupUi(this);
    layerManager = new LayerManager(this);
    transform = new TransformCRS(this);
    connect (ui->importBtn, &QPushButton::clicked, layerManager, &LayerManager::loadRasterLayer);
    //For displaying the CRSs list on the source and target Comboboxes
    setCrsList(ui->sourceCRSCombo);
    setCrsList(ui->targetCRSCombo);
    connect (ui->sourceCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSsource);
    connect (ui->targetCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSdest);
    listDimension(); //dimension pour afficher le contenu de la combobox
    carte = new Carte(ui->carte);
    connect(carte->getCanvas(),&QgsMapCanvas::scaleChanged, this,&MainWindow::updateScaleLabel);    
    connect (ui->btnZoomPlus, &QPushButton::clicked, this, &MainWindow::zoomIn_button);
    connect (ui->btnZoomMinus, &QPushButton::clicked, this, &MainWindow::zoomOut_button);
    //connect (ui->calendar, &QCalendarWidget::selectionChanged, this, &MainWindow::getDateSelected); 
      
    

    connect (ui->epochEdit, &QLineEdit::textEdited, transform, &TransformCRS::getDate);
    connect (ui->transformBtn, &QPushButton::clicked, transform, &TransformCRS::transform);

    connect (ui->addToMapBtn, &QPushButton::clicked, layerManager, &LayerManager::addFileToWidget);

    //When the "Nouveau" button is clicked, open a new window for choosing the CRS and the eopch
    connect (ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);
    //connect(ui->getDateSelected(), &QgsMapCanvas:: ,  this,&MainWindow::updateScaleLabel)
    //connect(this, &MainWindow::getDateSelected, this, &MainWindow::getDateSelected);

    //Dialog management
    dialog = new Dialog();
    connect (ui->layersList, &QListWidget::itemActivated, this, &MainWindow::openDialog);
    Ui::Dialog *dig = dialog -> getUI();
    connect(dig->buttonDuplicate, &QPushButton::clicked,
            this, [this]() {
                layerManager->duplicateLayer(dialog);
            });

    connect(dig->buttonRename, &QPushButton::clicked,
            this, [this]() {
                layerManager->renameLayer(dialog);
            });
    //connect (crsLabel)

}

MainWindow::~MainWindow()
{
    delete layerManager;
    delete ui;
}

Ui::MainWindow* MainWindow::getUi(){
    return ui;
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


void MainWindow::listDimension(){

    ui->dimensionCombo->clear();
    ui->dimensionCombo->addItem("2D");
    ui->dimensionCombo->addItem("4D");

}


//Open dialog when the layer is clicked
void MainWindow::openDialog() {
    dialog -> show();
}

void MainWindow::getDateSelected(const QDate &date){
    //QDate initalDate= ui->calendar->selectedDate();
    ui->date->setText("Date : " + date.toString("dd/MM/yyyy"));

}

void MainWindow::getSRCSelected(){
    ui->crsLabel->setText("CRS : " + ui->sourceCRSCombo->currentText());
}

//The function to set the CRS and the epoch of a new project when clicking on "Nouveau"
void MainWindow::setNewProject(){
    //Creating new project

    //Creation of the dialog window
    QDialog chosingCRSDialog;
    
    
    chosingCRSDialog.setWindowTitle("Nouveau projet");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);
    QLabel *dialogText = new QLabel("Choisissez un CRS et une époque pour votre projet", &chosingCRSDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);
    
    //Widget for choosing the name of the project
    QLineEdit *nameTextZone = new QLineEdit(&chosingCRSDialog);
    nameTextZone->setPlaceholderText("Entrez le nom du projet");
    
    //Widget for choosing the CRS of the project
    QComboBox *crsList = new QComboBox(&chosingCRSDialog);
    crsList->setPlaceholderText("Entrez le code EPSG du CRS");
    setCrsList(crsList);
    
    //Widget for choosing the epoch0 of the project by entering the exact geodectic date
    QLineEdit *epochTextZone = new QLineEdit(&chosingCRSDialog);
    epochTextZone->setPlaceholderText("Entrez l'époque");
    
    //Widget for choosing the epoch0 of the project by selecting the date on a calendar
    QCalendarWidget *calendar= new QCalendarWidget(&chosingCRSDialog);
    
    //The validator for blocking the user to enter a wrong date//TO FIX
    QDoubleValidator *doubleValidator = new QDoubleValidator(&chosingCRSDialog);
    //Setting range and decimals for the validator
    doubleValidator->setRange(0, 2030, 3);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    //Integrating the validator on the textzone
    epochTextZone->setValidator(doubleValidator);
    
    QLabel *decimalDate = new QLabel("Date décimale : ", &chosingCRSDialog);   
    getCalendarDays(calendar, decimalDate);
     
    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);

    connect(calendar, &QCalendarWidget::selectionChanged, this, [this, calendar]() {
        QDate selectedDate = calendar->selectedDate();
        this->getDateSelected(selectedDate);
    });
    connect (crsList, &QComboBox::currentTextChanged, this, [this, crsList] () {
        ui->crsLabel->setText("CRS : " + crsList->currentText());
    
    //connect (crsList, &QComboBox::currentTextChanged, this, &MainWindow::getSRCSelected);
    });

    //Laying all widgets on the layout
    layout->addWidget(dialogText);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(calendar);
    
    layout->addWidget(decimalDate);
    layout->addWidget(acceptationButton);

  

    chosingCRSDialog.exec();
    
    //for the moment, an empty project
    Project* newProject = new Project("test_projet", 1950.0);


    currentProject = newProject;

    std::cout << newProject->getName();
    std::cout << newProject->getEpoch0();

}

void MainWindow::getCalendarDays(QCalendarWidget *calendar, QLabel *decimalDate){
    {
        QDate initalDate= calendar->selectedDate();
        float initalValue = computeDate (initalDate.day(),
                                          initalDate.month(),
                                          initalDate.year());
        decimalDate->setText("Date décimale : "+ QString::number(initalValue, 'f', 6));

        connect (calendar,&QCalendarWidget::selectionChanged, 
                [calendar, decimalDate, this](){;
        
            QDate date = calendar->selectedDate();
            float dec = computeDate(date.day(), date.month(), date.year());
            decimalDate->setText("Date décimale :" + QString::number(dec, 'f', 6));
        
    });}
    
    
        
}

float MainWindow::computeDate(int day, int month, int year){
    
    QDate start (year,1,1);
    QDate selected (year, month, day);
    float daysCount = start.daysTo(selected)+1;
    float daysInYear = QDate::isLeapYear(year) ? 366 : 365;
    float deci_date = year + (daysCount-1)/(daysInYear);
    return deci_date;
 
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
            "RGM23 (10673)",
            
        };
        comboBox->addItems(items);
}
