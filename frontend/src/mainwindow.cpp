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
#include <QDebug>
#include "../include/Carte.h"

#include <core/Project.hpp>

#include <QMessageBox>
#include <QDir>

#include <core/DataManager.hpp>
#include <core/VectorLayer.hpp>
#include <qgsvectorlayer.h>
#include <qgsfield.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsproject.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    layerManager = new LayerManager(this);
    transform = new TransformCRS(this);
    connect(ui->importBtn, &QPushButton::clicked, layerManager, &LayerManager::listFiles);
    // For displaying the CRSs list on the source and target Comboboxes
    setCrsList(ui->sourceCRSCombo);
    setCrsList(ui->targetCRSCombo);

    // Dialog management
    connect(ui->sourceCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSsource);
    connect(ui->targetCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSdest);
    listDimension(); // dimension pour afficher le contenu de la combobox
    carte = new Carte(ui->carte, this);
    connect(carte->getCanvas(), &QgsMapCanvas::scaleChanged, this, &MainWindow::updateScaleLabel);
    connect(ui->btnZoomPlus, &QPushButton::clicked, this, &MainWindow::zoomIn_button);
    connect(ui->btnZoomMinus, &QPushButton::clicked, this, &MainWindow::zoomOut_button);
    // connect (ui->calendar, &QCalendarWidget::selectionChanged, this, &MainWindow::getDateSelected);

    connect(ui->epochEdit, &QLineEdit::textEdited, transform, &TransformCRS::getDate);
    connect(ui->transformBtn, &QPushButton::clicked, transform, &TransformCRS::transform);

    connect(ui->addToMapBtn, &QPushButton::clicked, layerManager, &LayerManager::addFileToWidget);

    // When the "Nouveau" button is clicked, open a new window for choosing the CRS and the eopch
    connect(ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);
    // connect(ui->getDateSelected(), &QgsMapCanvas:: ,  this,&MainWindow::updateScaleLabel)
    // connect(this, &MainWindow::getDateSelected, this, &MainWindow::getDateSelected);

    // Dialog management
    dialog = new Dialog();
    connect(ui->layersList, &QListWidget::itemActivated, this, &MainWindow::openDialog);
    Ui::Dialog *dig = dialog->getUI();
    connect(dig->buttonDuplicate, &QPushButton::clicked,
            this, [this]()
            { layerManager->duplicateLayer(dialog); });

    connect(dig->buttonRename, &QPushButton::clicked,
            this, [this]()
            { layerManager->renameLayer(dialog); });

    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveProject);
    connect(ui->btnOpen, &QPushButton::clicked, this, [this]()
            { loadProject(); });
    // connect (crsLabel)
}

MainWindow::~MainWindow()
{
    delete layerManager;
    delete ui;
}

Ui::MainWindow *MainWindow::getUi()
{
    return ui;
}

void MainWindow::updateScaleLabel(int scaleValue)
{
    ui->scale->setText(QString("Échelle : 1:%1").arg(QString::number((std::round(scaleValue / 100) * 100), 'f', 0)));
}

void MainWindow::zoomIn_button()
{
    carte->getCanvas()->zoomIn();
}

void MainWindow::zoomOut_button()
{
    carte->getCanvas()->zoomOut();
}

void MainWindow::listDimension()
{

    ui->dimensionCombo->clear();
    ui->dimensionCombo->addItem("2D");
    ui->dimensionCombo->addItem("4D");
}

// Open dialog when the layer is clicked
void MainWindow::openDialog()
{
    dialog->show();
}

LayerManager *MainWindow::getLayerManager()
{
    return layerManager;
}

Carte *MainWindow::getCarte()
{
    return carte;
}

void MainWindow::getDateSelected(const QDate &date)
{
    // QDate initalDate= ui->calendar->selectedDate();
    ui->date->setText("Date : " + date.toString("dd/MM/yyyy"));
}

void MainWindow::getSRCSelected()
{
    ui->crsLabel->setText("CRS : " + ui->sourceCRSCombo->currentText());
}

Project *MainWindow::getCurrentProject() { return currentProject; }

void MainWindow::setNewProject()
{
    // Création de la fenêtre de dialogue
    QDialog chosingCRSDialog;
    chosingCRSDialog.setWindowTitle("Nouveau projet");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);

    QLabel *dialogText = new QLabel("Choisissez un CRS et une époque pour votre projet", &chosingCRSDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);

    // Zone de texte pour le nom du projet
    QLineEdit *nameTextZone = new QLineEdit(&chosingCRSDialog);
    nameTextZone->setPlaceholderText("Entrez le nom du projet");

    // ComboBox pour le CRS
    QComboBox *crsList = new QComboBox(&chosingCRSDialog);
    crsList->setPlaceholderText("Entrez le code EPSG du CRS");
    setCrsList(crsList);

    // Validation pour l'époque
    QDoubleValidator *doubleValidator = new QDoubleValidator(&chosingCRSDialog);
    QLineEdit *epochTextZone = new QLineEdit(&chosingCRSDialog);
    epochTextZone->setPlaceholderText("Entrez l'époque");

    // Validation pour l'époque
    doubleValidator->setRange(0, 2030, 3);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    epochTextZone->setValidator(doubleValidator);

    // Calendrier pour choisir la date
    QCalendarWidget *calendar = new QCalendarWidget(&chosingCRSDialog);
    QLabel *decimalDate = new QLabel("Date décimale : ", &chosingCRSDialog);
    getCalendarDays(calendar, decimalDate);

    // Connexions
    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);

    connect(calendar, &QCalendarWidget::selectionChanged, this, [this, calendar]()
            {
        QDate selectedDate = calendar->selectedDate();
        this->getDateSelected(selectedDate); });

    connect(crsList, &QComboBox::currentTextChanged, this, [this, crsList]()
            { ui->crsLabel->setText("CRS : " + crsList->currentText()); });

    // Layout
    layout->addWidget(dialogText);
    layout->addWidget(nameTextZone);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(calendar);
    layout->addWidget(decimalDate);
    layout->addWidget(acceptationButton);

    // Exécution du dialogue
    if (chosingCRSDialog.exec() != QDialog::Accepted)
    {
        qDebug() << "Création de projet annulée.";
        return;
    }

    // Récupérer les valeurs saisies par l'utilisateur
    QString projectName = nameTextZone->text().trimmed();
    QString selectedCrs = crsList->currentText().trimmed();
    double epoch = epochTextZone->text().toDouble();

    // Valeurs par défaut si l'utilisateur n'a rien saisi
    if (projectName.isEmpty())
        projectName = "ProjetSansNom";
    if (selectedCrs.isEmpty())
    {
        selectedCrs = "EPSG:2154"; // CRS par défaut
    }
    else
    {
        // Extraire le code EPSG si le format est "Nom (xxxx)"
        QRegExp rx("\\((\\d+)\\)");
        if (rx.indexIn(selectedCrs) != -1)
        {
            QString code = rx.cap(1);
            selectedCrs = "EPSG:" + code; // Convertit en format standard EPSG
        }
    }

    // Epoch par défaut si non valide
    if (epoch <= 0)
        epoch = 0.0;

    // Création d'un nouveau projet avec les valeurs saisies
    Project *newProject = new Project(
        projectName.toStdString(), // Nom du projet
        epoch,                     // Époque
        selectedCrs.toStdString()  // CRS (format "EPSG:xxxx")
    );

    // Stocker dans currentProject
    currentProject = newProject;

    qDebug() << "Nom du projet :" << projectName;
    qDebug() << "CRS sélectionné :" << selectedCrs;
    qDebug() << "Époque :" << epoch;

    qDebug() << "Nouveau projet créé :";
    qDebug() << "  Nom :" << QString::fromStdString(currentProject->getName());
    qDebug() << "  CRS :" << QString::fromStdString(currentProject->getCrs());
    qDebug() << "  Époque :" << currentProject->getEpoch0();
}

void MainWindow::getCalendarDays(QCalendarWidget *calendar, QLabel *decimalDate)
{
    {
        QDate initalDate = calendar->selectedDate();
        float initalValue = computeDate(initalDate.day(),
                                        initalDate.month(),
                                        initalDate.year());
        decimalDate->setText("Date décimale : " + QString::number(initalValue, 'f', 6));

        connect(calendar, &QCalendarWidget::selectionChanged,
                [calendar, decimalDate, this]()
                {
                    ;

                    QDate date = calendar->selectedDate();
                    float dec = computeDate(date.day(), date.month(), date.year());
                    decimalDate->setText("Date décimale :" + QString::number(dec, 'f', 6));
                });
    }
}

float MainWindow::computeDate(int day, int month, int year)
{

    QDate start(year, 1, 1);
    QDate selected(year, month, day);
    float daysCount = start.daysTo(selected) + 1;
    float daysInYear = QDate::isLeapYear(year) ? 366 : 365;
    float deci_date = year + (daysCount - 1) / (daysInYear);
    return deci_date;
}

// Function to set the targetted comboBox to show the list of CRS accepted by the project
void MainWindow::setCrsList(QComboBox *comboBox)
{
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
        "RGF93v1 (2154)",

    };
    comboBox->addItems(items);
}

void MainWindow::saveProject()
{
    // Check if a project exists
    if (currentProject == nullptr)
    {
        QMessageBox::warning(this, "Error", "No project to save. Create a project first with 'New'.");
        return;
    }

    // Ask user where to save the project
    QString filepath = QFileDialog::getSaveFileName(
        this,
        tr("Save Project"),
        "/home/user/" + QString::fromStdString(currentProject->getName()) + ".sigm4",
        tr("SIGM4 Files (*.sigm4)"));

    // If user cancels the dialog
    if (filepath.isEmpty())
    {
        return;
    }

    // Save the project using the backend
    bool success = currentProject->save(filepath.toStdString());

    // Show success or error message
    if (success)
    {
        QMessageBox::information(this, "Success", "Project saved successfully!");
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to save project.");
    }
}

// No parameters - for the "Open" button in UI
void MainWindow::loadProject()
{
    // Ask user which file to load via file dialog
    QString filepath = QFileDialog::getOpenFileName(
        this,
        tr("Open Project"),
        "/home/user/",
        tr("SIGM4 Files (*.sigm4)"));

    // If user cancels the dialog
    if (filepath.isEmpty())
    {
        return;
    }

    // Call the overloaded version with the filepath
    loadProject(filepath);
}

// With filepath parameter - for command-line/file association
void MainWindow::loadProject(const QString &filepath)
{
    // Validate filepath before attempting to load
    if (filepath.isEmpty() || !QFile::exists(filepath))
    {
        qWarning() << "Invalid filepath:" << filepath;
        return;
    }

    try
    {
        // Load project from backend using the Project class
        Project loadedProject = Project::load(filepath.toStdString());

        // Delete existing project if one exists
        if (currentProject != nullptr)
        {
            delete currentProject;
        }

        // Create new project instance with loaded data
        currentProject = new Project(
            loadedProject.getName(),
            loadedProject.getEpoch0(),
            loadedProject.getCrs(),
            loadedProject.getLayers());

        // Update UI with project information
        ui->crsLabel->setText("CRS : " + QString::fromStdString(currentProject->getCrs()));

        // Update date label from epoch
        double epoch = currentProject->getEpoch0();
        int year = static_cast<int>(epoch);
        double fractionalYear = epoch - year;
        int dayOfYear = static_cast<int>(fractionalYear * 365);
        QDate projectDate = QDate(year, 1, 1).addDays(dayOfYear);
        ui->date->setText("Date : " + projectDate.toString("dd/MM/yyyy"));

        // RELOAD LAYERS

        // Clear the layers list in the UI
        ui->layersList->clear();

        // Get layers from the project
        std::vector<Layer> layers = currentProject->getLayers();

        qDebug() << "Reloading" << layers.size() << "layer(s)...";

        // Reload each layer
        for (const auto &layer : layers)
        {
            QString layerName = QString::fromStdString(layer.getName());
            QString dataSource = QString::fromStdString(layer.getDataSource());

            qDebug() << "Loading layer:" << layerName;
            qDebug() << "  Source:" << dataSource;

            // Add layer to the layers list widget
            QListWidgetItem *item = new QListWidgetItem(layerName);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            ui->layersList->addItem(item);

            // If source file exists, reload the data
            if (!dataSource.isEmpty() && QFile::exists(dataSource))
            {
                try
                {
                    // Use DataManager to reload the layer
                    DataManager dm;
                    std::vector<VectorLayer *> reloadedLayers = dm.loadVector(dataSource.toStdString());

                    // Find the corresponding layer
                    for (auto *vLayer : reloadedLayers)
                    {
                        if (vLayer->getName() == layer.getName())
                        {
                            // Create QgsVectorLayer for display
                            QString qlayerName = QString::fromStdString(vLayer->getName());
                            QgsVectorLayer *qlayer = new QgsVectorLayer(
                                "Point?crs=" + QString::fromStdString(vLayer->getCrs()),
                                qlayerName,
                                "memory");

                            // Add fields to the layer
                            QList<QgsField> fieldList;
                            fieldList << QgsField("id", QVariant::Int);
                            qlayer->dataProvider()->addAttributes(fieldList);
                            qlayer->updateFields();

                            // Add geometries from EWKT format
                            auto ewkts = vLayer->getEWKT();
                            int fid = 0;
                            for (const auto &ewkt : ewkts)
                            {
                                if (ewkt.empty())
                                    continue;

                                // Remove "SRID=xxxx;" prefix
                                std::string wkt = ewkt;
                                size_t pos = ewkt.find(';');
                                if (pos != std::string::npos)
                                {
                                    wkt = ewkt.substr(pos + 1);
                                }

                                // Create QGIS geometry from WKT
                                QgsGeometry qgsGeom = QgsGeometry::fromWkt(QString::fromStdString(wkt));
                                if (!qgsGeom.isEmpty())
                                {
                                    QgsFeature feat;
                                    feat.setGeometry(qgsGeom);
                                    feat.setAttributes({fid++});
                                    qlayer->dataProvider()->addFeature(feat);
                                }
                            }

                            qlayer->updateExtents();

                            // Add to QGIS project
                            QgsProject::instance()->addMapLayer(qlayer, false);

                            // Place layer above basemap
                            Carte *carte = getCarte();
                            QgsMapCanvas *canvas = carte->getCanvas();
                            auto currentLayers = canvas->layers();
                            currentLayers.prepend(qlayer);
                            canvas->setLayers(currentLayers);
                            canvas->refresh();

                            qDebug() << "Layer reloaded with" << fid << "geometries";
                            break;
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    qDebug() << "Error reloading:" << e.what();
                }
            }
            else
            {
                qDebug() << "Source file not found:" << dataSource;
            }
        }

        // Zoom to fit all vector layers
        Carte *carte = getCarte();
        QgsMapCanvas *canvas = carte->getCanvas();
        QList<QgsMapLayer *> layerList = canvas->layers();

        QgsRectangle extent;
        bool first = true;
        for (auto *layer : layerList)
        {
            QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>(layer);
            if (vectorLayer)
            {
                if (first)
                {
                    extent = vectorLayer->extent();
                    first = false;
                }
                else
                {
                    extent.combineExtentWith(vectorLayer->extent());
                }
            }
        }
        if (!first)
        {
            canvas->setExtent(extent);
        }

        canvas->refresh();

        // Show success message
        QMessageBox::information(
            this,
            "Success",
            QString("Project '%1' loaded successfully!\n\nCRS: %2\nEpoch: %3\nNumber of layers: %4")
                .arg(QString::fromStdString(currentProject->getName()))
                .arg(QString::fromStdString(currentProject->getCrs()))
                .arg(currentProject->getEpoch0())
                .arg(currentProject->getLayers().size()));
    }
    catch (const std::exception &e)
    {
        // Handle loading errors
        QMessageBox::critical(
            this,
            "Error",
            QString("Failed to load project:\n%1").arg(e.what()));
    }
}