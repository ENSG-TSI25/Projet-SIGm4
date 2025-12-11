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
    , projectDisplay(new ProjectCarateristicsDisplay(this))
{
    ui->setupUi(this);
    layerManager = new LayerManager(this);
    transform = new TransformCRS(this);
    connect (ui->importBtn, &QPushButton::clicked, layerManager, &LayerManager::listFiles);
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

    connect (ui->addToMapBtn, &QPushButton::clicked, layerManager, &LayerManager::addFileToWidget);

    //When the "Nouveau" button is clicked, open a new window for choosing the CRS and the eopch
    connect (ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);

    //When the "Ouvrir" button is clicked, open the file manager to choose a new project to open
    connect (ui->btnOpen, &QPushButton::clicked, this, &MainWindow::openExistingProject);

    //When the "Enregistrer" button is clicked, open the file manager to choose the saving location
    connect (ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveCurrentProject);

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
    
    //To show the careteristics of the current project
    ui->projectCaracteristicsDisplay->addWidget(projectDisplay);

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

// void MainWindow::displayEpochProject(const QDate &date){
//     ui->epochDisplayZone->setText("Époque du projet: " + date.toString("dd/MM/yyyy"));
// }

// void MainWindow::getSRCSelected(){
//     ui->crsLabel->setText("CRS : " + ui->sourceCRSCombo->currentText());
// }

//The function to set the name, the CRS and the epoch of a new project when clicking on "Nouveau"
void MainWindow::setNewProject(){

    //Creation of the dialog window
    QDialog newProjectDialog;
    
    
    newProjectDialog.setWindowTitle("Nouveau projet");
    QVBoxLayout *layout = new QVBoxLayout(&newProjectDialog);
    QLabel *dialogText = new QLabel("Choisissez les caractéristiques de votre nouveau projet", &newProjectDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &newProjectDialog);
    
    //Widget for choosing the name of the project
    QLineEdit *nameTextZone = new QLineEdit(&newProjectDialog);
    nameTextZone->setPlaceholderText("Entrez le nom du projet");
    
    //Widget for choosing the CRS of the project
    QComboBox *crsList = new QComboBox(&newProjectDialog);
    crsList->setPlaceholderText("Entrez le code EPSG du CRS");
    setCrsList(crsList);
    
    //Widget for choosing the epoch0 of the project by entering the exact geodectic date
    QLineEdit *epochTextZone = new QLineEdit(&newProjectDialog);
    epochTextZone->setPlaceholderText("Entrez l'époque");
    
    //Widget for choosing the epoch0 of the project by selecting the date on a calendar
    QCalendarWidget *calendar= new QCalendarWidget(&newProjectDialog);
    
    //The validator for blocking the user to enter a wrong date//TO FIX
    QDoubleValidator *doubleValidator = new QDoubleValidator(&newProjectDialog);
    //Setting range and decimals for the validator
    doubleValidator->setRange(1950, 2030, 3);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    //Integrating the validator on the textzone
    epochTextZone->setValidator(doubleValidator);
     
    QObject::connect(acceptationButton, &QPushButton::clicked, &newProjectDialog, &QDialog::accept);

    connect(calendar, &QCalendarWidget::selectionChanged, this, [this, calendar]() {
        QDate selectedDate = calendar->selectedDate();
    });

    // connect (crsList, &QComboBox::currentTextChanged, this, [this, crsList] () {
    //     ui->crsLabel->setText("CRS : " + crsList->currentText());
    // });


    // connect the calendar with the epoch textzone
    connect(calendar, &QCalendarWidget::selectionChanged, this, [calendar, epochTextZone]() {
        QDate selectedDate = calendar->selectedDate();
        // epoch calculated from the date 
        double decimalYear = selectedDate.year() + 
                            (selectedDate.dayOfYear() - 1) / 
                            (selectedDate.isLeapYear(selectedDate.year()) ? 366.0 : 365.0);
        epochTextZone->setText(QString::number(decimalYear, 'f', 3));
    });

    //Laying all widgets on the layout
    layout->addWidget(dialogText);
    layout->addWidget(nameTextZone);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(calendar);
    layout->addWidget(acceptationButton);

    // check if the user has created the project
    if (newProjectDialog.exec() == QDialog::Accepted) {

        // get the name 
        QString projectName = nameTextZone->text();
        if (projectName.isEmpty()) {
            projectName = "Projet sans nom"; 
        }

        // get the CRS 
        QString selectedCRS = crsList->currentText();

        //EPSG code 
        int startIndex = selectedCRS.indexOf('(');
        int endIndex = selectedCRS.indexOf(')');
        QString epsgCode;
        
        if (startIndex != -1 && endIndex != -1) {
            epsgCode = "EPSG:" + selectedCRS.mid(startIndex + 1, endIndex - startIndex - 1);
        } else {
            epsgCode = "EPSG:4326";  //  default value
        }
        
        //epoch
        double epoch = epochTextZone->text().toDouble();
        
        Project* newProject = new Project(
            projectName.toStdString(),     // name
            epoch,                         // epoch
            epsgCode.toStdString()        // CRS
        );
        currentProject = newProject;
        
        //Setting the parameters for the display
        projectDisplay->setProjectName(projectName.toStdString());
        projectDisplay->setProjectCRS(epsgCode.toStdString());
        projectDisplay->setProjectEpoch0(epoch);

        // cout
        std::cout << "Projet créé avec succès !" << std::endl;
        std::cout << "Nom : " << newProject->getName() << std::endl;
        std::cout << "Époque : " << newProject->getEpoch0() << std::endl;
        std::cout << "CRS : " << newProject->getCrs() << std::endl;
        
    } else {
        std::cout << "Création du projet annulée" << std::endl;
        currentProject = nullptr;
    }

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
        
            });
    }    
}

//Function to compute the calendar date into decimal date
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

// Function to open an already existing project

    void MainWindow::openExistingProject(){

        std::cout << "début ouverture projet" << std::endl;
    QDialog openProjectDialog;
    openProjectDialog.setWindowTitle("Ouvrir un projet");
    
    QVBoxLayout *layout = new QVBoxLayout(&openProjectDialog);
    
    QLabel *dialogText = new QLabel("Sélectionnez le fichier projet à ouvrir", &openProjectDialog);
    
    // Widget file path 
    QHBoxLayout *filePathLayout = new QHBoxLayout();
    
    QLineEdit *pathTextZone = new QLineEdit(&openProjectDialog);
    pathTextZone->setPlaceholderText("Chemin du fichier...");
    
    QPushButton *browseButton = new QPushButton("Parcourir", &openProjectDialog);
    
    filePathLayout->addWidget(pathTextZone);
    filePathLayout->addWidget(browseButton);
    
    QPushButton *acceptationButton = new QPushButton("OK", &openProjectDialog);
    QPushButton *cancelButton = new QPushButton("Annuler", &openProjectDialog);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(acceptationButton);

    //Add the widget to the layout
    layout->addWidget(dialogText);
    layout->addLayout(filePathLayout);
    layout->addLayout(buttonLayout);
    
    // connection to the buttons
    connect(browseButton, &QPushButton::clicked, [&]() {
        QString filePath = QFileDialog::getOpenFileName(
            &openProjectDialog,
            "Sélectionner un fichier projet",
            QDir::homePath(),
            "Fichiers projet (*.proj *.xml);;Tous les fichiers (*.*)"
        );
        
        if (!filePath.isEmpty()) {
            pathTextZone->setText(filePath);
        }
    });

    connect(acceptationButton, &QPushButton::clicked, &openProjectDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &openProjectDialog, &QDialog::reject);
    
    // Dialog
    if (openProjectDialog.exec() == QDialog::Accepted) {
        QString selectedPath = pathTextZone->text();
        
        if (!selectedPath.isEmpty()) {
            // Verify the file existence
            QFileInfo fileInfo(selectedPath);
            if (fileInfo.exists()) {
                std::cout << "Ouverture du fichier : " << selectedPath.toStdString() << std::endl;
                
                // TODO: Charger le projet depuis le fichier
                // loadProjectFromFile(selectedPath);
                
            } else {
                std::cerr << "Erreur : le fichier n'existe pas" << std::endl;
            }
        }
    } else {
        std::cout << "Ouverture annulée" << std::endl;
    }
    }

// Function to open an already existing project 

    void MainWindow::saveCurrentProject(){
        std::cout << "fermeture sauvegarde projet" << std::endl;
        // Création de la fenêtre de dialogue
    QDialog saveProjectDialog;
    saveProjectDialog.setWindowTitle("Enregistrer le projet");
    
    QVBoxLayout *layout = new QVBoxLayout(&saveProjectDialog);
    
    QLabel *dialogText = new QLabel("Choisissez l'emplacement de sauvegarde", &saveProjectDialog);
    
    // file path widget
    QHBoxLayout *filePathLayout = new QHBoxLayout();
    
    QLineEdit *pathTextZone = new QLineEdit(&saveProjectDialog);
    pathTextZone->setPlaceholderText("Chemin du fichier...");
    
    // Set a default name if a project is open
    if (currentProject != nullptr) {
        pathTextZone->setText(QString::fromStdString(currentProject->getName()) + ".proj");
    }
    
    QPushButton *browseButton = new QPushButton("Parcourir", &saveProjectDialog);
    
    filePathLayout->addWidget(pathTextZone);
    filePathLayout->addWidget(browseButton);
    
    QPushButton *acceptationButton = new QPushButton("Enregistrer", &saveProjectDialog);
    QPushButton *cancelButton = new QPushButton("Annuler", &saveProjectDialog);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(acceptationButton);
    
    // add the widget to the layout
    layout->addWidget(dialogText);
    layout->addLayout(filePathLayout);
    layout->addLayout(buttonLayout);
    
    // connecting the buttons
    connect(browseButton, &QPushButton::clicked, [&]() {
        QString filePath = QFileDialog::getSaveFileName(
            &saveProjectDialog,
            "Enregistrer le projet",
            pathTextZone->text().isEmpty() ? QDir::homePath() : pathTextZone->text(),
            "Fichiers projet (*.proj);;Fichiers XML (*.xml)"
        );
        
        if (!filePath.isEmpty()) {
            pathTextZone->setText(filePath);
        }
    });
    
    // connexion
    connect(acceptationButton, &QPushButton::clicked, &saveProjectDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &saveProjectDialog, &QDialog::reject);
    
    // Dialog
    if (saveProjectDialog.exec() == QDialog::Accepted) {
        QString selectedPath = pathTextZone->text();
        
        if (!selectedPath.isEmpty()) {
            std::cout << "Sauvegarde du projet à : " << selectedPath.toStdString() << std::endl;
            
            // TODO: Sauvegarder le projet dans le fichier
            // saveProjectToFile(selectedPath);
            
        } else {
            std::cerr << "Erreur : chemin vide" << std::endl;
        }
    } else {
        std::cout << "Sauvegarde annulée" << std::endl;
    }
    }