#include "../include/mainwindow.h"
#include "../include/Layer.h"
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
    layer = new Layer(this);
    transform = new TransformCRS(this);
    connect (ui->importBtn, &QPushButton::clicked, layer, &Layer::listFiles);
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
      
    

    connect (ui->epochEdit, &QLineEdit::textEdited, transform, &TransformCRS::getDate);
    connect (ui->transformBtn, &QPushButton::clicked, transform, &TransformCRS::transform);

    connect (ui->addToMapBtn, &QPushButton::clicked, layer, &Layer::addFileToWidget);

    //When the "Nouveau" button is clicked, open a new window for choosing the CRS and the eopch
    connect (ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);

    //Dialog management
    dialog = new Dialog();
    connect (ui->layersList, &QListWidget::itemActivated, this, &MainWindow::openDialog);
    Ui::Dialog *dig = dialog -> getUI();
    connect(dig->buttonDuplicate, &QPushButton::clicked,
            this, [this]() {
                layer->duplicateLayer(dialog);
            });

    connect(dig->buttonRename, &QPushButton::clicked,
            this, [this]() {
                layer->renameLayer(dialog);
            });


}

MainWindow::~MainWindow()
{
    delete layer;
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

//The function to set the CRS and the epoch of a new project when clicking on "Nouveau"
void MainWindow::setNewProject(){
    //Creation of the dialog window
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
    QCalendarWidget *calendar= new QCalendarWidget(&chosingCRSDialog);
    
    

    //On crée un validateur pour vérifier que l'utilisateur ne rentre bien que des doubles
    QDoubleValidator *doubleValidator = new QDoubleValidator(&chosingCRSDialog);
    //range and decimals
    doubleValidator->setRange(0, 2030, 3);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    //On intègre le validateur à la zone de texte
    epochTextZone->setValidator(doubleValidator);




    //on ajoute les widgets
    layout->addWidget(dialogText);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(calendar);
    layout->addWidget(acceptationButton);
    


    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);

    chosingCRSDialog.exec();
}

/*void MainWindow::getCalendarDays(QCalendarWidget *calendarwidget){
    connect (calendar,&QCalendarWidget::selectionChanged, [=](){
        QDate date = calendar->selectedDate();
        int day = date.day();
        int month = date.month();
        int year = date.year();
        
    });
    
}*/


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
