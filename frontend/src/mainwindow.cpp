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

/**
 * @file mainwindow.cpp
 * @brief Implementation of MainWindow class
 * 
 * Contains main application logic, UI initialization,
 * and event handling for the GIS application.
 */

/**
 * @brief Constructor - initializes main window
 * @param parent Parent widget
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) 
{
    ui->setupUi(this);
    layerManager = new LayerManager(this);
    transform = new TransformCRS(this);
    
    connect(ui->importBtn, &QPushButton::clicked, layerManager, &LayerManager::listFiles);
    setCrsList(ui->sourceCRSCombo);
    setCrsList(ui->targetCRSCombo);
    connect(ui->sourceCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSsource);
    connect(ui->targetCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSdest);
    
    listDimension();
    carte = new Carte(ui->carte);
    connect(carte->getCanvas(), &QgsMapCanvas::scaleChanged, this, &MainWindow::updateScaleLabel);    
    connect(ui->btnZoomPlus, &QPushButton::clicked, this, &MainWindow::zoomIn_button);
    connect(ui->btnZoomMinus, &QPushButton::clicked, this, &MainWindow::zoomOut_button);
    
    connect(ui->epochEdit, &QLineEdit::textEdited, transform, &TransformCRS::getDate);
    connect(ui->transformBtn, &QPushButton::clicked, transform, &TransformCRS::transform);
    connect(ui->addToMapBtn, &QPushButton::clicked, layerManager, &LayerManager::addFileToWidget);
    
    connect(ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);
    
    dialog = new Dialog();
    connect(ui->layersList, &QListWidget::itemActivated, this, &MainWindow::openDialog);
    Ui::Dialog *dig = dialog->getUI();
    
    connect(dig->buttonDuplicate, &QPushButton::clicked,
            this, [this]() {
                layerManager->duplicateLayer(dialog);
            });
    
    connect(dig->buttonRename, &QPushButton::clicked,
            this, [this]() {
                layerManager->renameLayer(dialog);
            });
}

/**
 * @brief Destructor
 */
MainWindow::~MainWindow()
{
    delete layerManager;
    delete ui;
}

/**
 * @brief Gets the UI object
 * @return Pointer to main window UI
 */
Ui::MainWindow* MainWindow::getUi(){
    return ui;
}

/**
 * @brief Updates scale label on map zoom
 * @param scaleValue Current map scale
 */
void MainWindow::updateScaleLabel(int scaleValue)
{
    ui->scale->setText(QString("Échelle : 1:%1").arg(QString::number((std::round(scaleValue/100)*100), 'f', 0)));
}

/**
 * @brief Zoom in button handler
 */
void MainWindow::zoomIn_button()
{
    carte->getCanvas()->zoomIn();
}

/**
 * @brief Zoom out button handler
 */
void MainWindow::zoomOut_button()
{
    carte->getCanvas()->zoomOut();
}

/**
 * @brief Populates dimension combo box
 */
void MainWindow::listDimension(){
    ui->dimensionCombo->clear();
    ui->dimensionCombo->addItem("2D");
    ui->dimensionCombo->addItem("4D");
}

/**
 * @brief Opens layer management dialog
 */
void MainWindow::openDialog() {
    dialog->show();
}

/**
 * @brief Handles date selection from calendar
 * @param date Selected date
 */
void MainWindow::getDateSelected(const QDate &date){
    ui->date->setText("Date : " + date.toString("dd/MM/yyyy"));
}

/**
 * @brief Updates CRS label with selected coordinate system
 */
void MainWindow::getSRCSelected(){
    ui->crsLabel->setText("CRS : " + ui->sourceCRSCombo->currentText());
}

/**
 * @brief Creates new project dialog
 * 
 * Opens dialog for setting project name, CRS, and epoch.
 * Creates a new Project instance with user parameters.
 */
void MainWindow::setNewProject(){
    QDialog chosingCRSDialog;
    chosingCRSDialog.setWindowTitle("Nouveau projet");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);
    QLabel *dialogText = new QLabel("Choisissez un CRS et une époque pour votre projet", &chosingCRSDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);
    
    QLineEdit *nameTextZone = new QLineEdit(&chosingCRSDialog);
    nameTextZone->setPlaceholderText("Entrez le nom du projet");
    
    QComboBox *crsList = new QComboBox(&chosingCRSDialog);
    crsList->setPlaceholderText("Entrez le code EPSG du CRS");
    setCrsList(crsList);
    
    QLineEdit *epochTextZone = new QLineEdit(&chosingCRSDialog);
    epochTextZone->setPlaceholderText("Entrez l'époque");
    
    QCalendarWidget *calendar = new QCalendarWidget(&chosingCRSDialog);
    QDoubleValidator *doubleValidator = new QDoubleValidator(&chosingCRSDialog);
    doubleValidator->setRange(0, 2030, 3);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    epochTextZone->setValidator(doubleValidator);
    
    QLabel *decimalDate = new QLabel("Date décimale : ", &chosingCRSDialog);   
    getCalendarDays(calendar, decimalDate);
     
    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);

    connect(calendar, &QCalendarWidget::selectionChanged, this, [this, calendar]() {
        QDate selectedDate = calendar->selectedDate();
        this->getDateSelected(selectedDate);
    });
    
    connect(crsList, &QComboBox::currentTextChanged, this, [this, crsList]() {
        ui->crsLabel->setText("CRS : " + crsList->currentText());
    });

    layout->addWidget(dialogText);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(calendar);
    layout->addWidget(decimalDate);
    layout->addWidget(acceptationButton);

    chosingCRSDialog.exec();
    
    Project* newProject = new Project("test_projet", 1950.0);
    currentProject = newProject;

    std::cout << newProject->getName();
    std::cout << newProject->getEpoch0();
}

/**
 * @brief Connects calendar to decimal date display
 * @param calendar Calendar widget
 * @param decimalDate Label for decimal date display
 */
void MainWindow::getCalendarDays(QCalendarWidget *calendar, QLabel *decimalDate){
    QDate initalDate = calendar->selectedDate();
    float initalValue = computeDate(initalDate.day(),
                                    initalDate.month(),
                                    initalDate.year());
    decimalDate->setText("Date décimale : "+ QString::number(initalValue, 'f', 6));

    connect(calendar, &QCalendarWidget::selectionChanged, 
            [calendar, decimalDate, this]() {
                QDate date = calendar->selectedDate();
                float dec = computeDate(date.day(), date.month(), date.year());
                decimalDate->setText("Date décimale :" + QString::number(dec, 'f', 6));
            });
}

/**
 * @brief Computes decimal date from day/month/year
 * @param day Day of month
 * @param month Month (1-12)
 * @param year Year
 * @return Decimal date (year.fraction)
 */
float MainWindow::computeDate(int day, int month, int year){
    QDate start(year, 1, 1);
    QDate selected(year, month, day);
    float daysCount = start.daysTo(selected) + 1;
    float daysInYear = QDate::isLeapYear(year) ? 366 : 365;
    float deci_date = year + (daysCount - 1) / (daysInYear);
    return deci_date;
}

/**
 * @brief Populates combo box with CRS options
 * @param comboBox Combo box to populate
 */
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