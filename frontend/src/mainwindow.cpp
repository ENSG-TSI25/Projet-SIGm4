#include "../include/mainwindow.h"
#include "../include/LayerManager.h"
#include "../include/TransformCRS.h"
#include "../include/MapToolIdentifyFeature.h"

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
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QStringList>
#include <QListWidget>
#include <iostream>
#include <QDate>
#include <QDebug>
#include "../include/Carte.h"

#include <core/Project.hpp>
#include <QMessageBox>

#include <QMessageBox>
#include <QDir>
#include <qgsvectorlayercache.h>
#include <qgsattributetablemodel.h>
#include <qgsattributetablefiltermodel.h>
#include <qgsattributetableview.h>
#include <qgsmaplayer.h>


#include <core/DataManager.hpp>
#include <core/VectorLayer.hpp>
#include <qgsvectorlayer.h>
#include <qgsfield.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsproject.h>

#include <gdal_priv.h>    //Updating the display of the project

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , projectDisplay(new ProjectCarateristicsDisplay(this))
{
    ui->setupUi(this);
    layerManager = new LayerManager(this);
    transform = new TransformCRS(this);

    //Disable all actions while there is no active project
    setProjectActionsEnabled(false);

    connect(ui->importBtn, &QPushButton::clicked, layerManager, &LayerManager::listFiles);

    // For displaying the CRSs list on the source and target Comboboxes
    setCrsList(ui->sourceCRSCombo);
    setCrsList(ui->targetCRSCombo);

    connect (ui->sourceCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSsource);
    connect (ui->targetCRSCombo, &QComboBox::currentTextChanged, transform, &TransformCRS::selectCRSdest);


    listDimension(); //dimension pour afficher le contenu de la combobox
    carte = new Carte(ui->carte, this);
    // === Outil Identify (popup infos, attributs, coordonnées) ===

    ui->btnIdentify->setCheckable(true);
    ui->btnIdentify->setChecked(false);

    ui->btnIdentify->setStyleSheet(
    "QPushButton:checked { background-color: #3daee9; color: white; }"
);



    //     identifyTool =
    // new QgsMapToolIdentify(carte->getCanvas());
    identifyTool =
    new MapToolIdentifyFeature(
        carte->getCanvas(),
        this
    );

    defaultTool =
    new QgsMapToolPan(carte->getCanvas());
    carte->getCanvas()->setMapTool(defaultTool);
    connect(ui->btnIdentify, &QPushButton::toggled,
            this, [this](bool checked)
    {
        if (checked)
        {
            carte->getCanvas()->setMapTool(identifyTool);
        }
        else
        {
            carte->getCanvas()->setMapTool(defaultTool);
        }
    });




    connect(carte->getCanvas(),&QgsMapCanvas::scaleChanged, this,&MainWindow::updateScaleLabel);  
    connect(
        carte->getCanvas(),
        &QgsMapCanvas::xyCoordinates,
        this,
        [this](const QgsPointXY& p)
        {
            ui->scale->setText(
                QString("X: %1  Y: %2")
                    .arg(p.x(), 0, 'f', 3)
                    .arg(p.y(), 0, 'f', 3)
            );
        }
    );

    connect (ui->btnZoomPlus, &QPushButton::clicked, this, &MainWindow::zoomIn_button);
    connect (ui->btnZoomMinus, &QPushButton::clicked, this, &MainWindow::zoomOut_button);
      
    connect (ui->epochEdit, &QLineEdit::textEdited, transform, &TransformCRS::getDate);
    connect (ui->transformBtn, &QPushButton::clicked, transform, &TransformCRS::transform);

    //connect (ui->addToMapBtn, &QPushButton::clicked, layerManager, &LayerManager::loadRasterLayerFromFile);
    connect (ui->addToMapBtn, &QPushButton::clicked, layerManager, &LayerManager::addFileToWidget);

    //When the "Nouveau" button is clicked, open a new window for choosing the CRS and the eopch
    connect (ui->btnNew, &QPushButton::clicked, this, &MainWindow::setNewProject);

    //When the "Ouvrir" button is clicked, open the file manager to choose a new project to open
    connect(ui->btnOpen, &QPushButton::clicked, this, [this]()
            { loadProject(); });

    //When the "Enregistrer" button is clicked, open the file manager to choose the saving location
    connect(ui->btnSave, &QPushButton::clicked, this, &MainWindow::saveProject);

    //checkbox
    qDebug() << "++++++";
    connect (ui->layersList, &QListWidget::itemChanged, layerManager, &LayerManager::displayLayer);

    //Dialog management
    dialog = new Dialog();
    connect (ui->layersList, &QListWidget::itemDoubleClicked, this, &MainWindow::openDialog);
    Ui::Dialog *dig = dialog -> getUI();
    connect(dig->buttonDuplicate, &QPushButton::clicked,
            this, [this]()
            { layerManager->duplicateLayer(dialog); });

    connect(dig->buttonRename, &QPushButton::clicked,
            this, [this]() {
                layerManager->renameLayer(dialog);
            });
    
    //To show the careteristics of the current project
    ui->projectCaracteristicsDisplay->addWidget(projectDisplay);


    ui->layersList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->layersList, &QListWidget::customContextMenuRequested,
        this, [this](const QPoint& pos)
{
    QListWidgetItem* item =
    ui->layersList->currentItem();

    if (!item)
        return;

    QString layerId =
        item->data(Qt::UserRole).toString();

    QgsMapLayer* layer =
        QgsProject::instance()->mapLayer(layerId);

    if (!layer)
    {
        QMessageBox::warning(
            this,
            "Erreur",
            "Couche introuvable dans le projet."
        );
        return;
    }

    QgsVectorLayer* vlayer =
        qobject_cast<QgsVectorLayer*>(layer);

    if (!vlayer)
    {
        QMessageBox::warning(
            this,
            "Erreur",
            "La couche sélectionnée n'est pas vectorielle."
        );
        return;
    }

    openAttributeTable(vlayer);

});


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

void MainWindow::openAttributeTable(QgsVectorLayer* layer)
{
    if (!layer) return;

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Table attributaire");
    dlg->resize(900, 500);

    QVBoxLayout* layout = new QVBoxLayout(dlg);

    // Cache (OBLIGATOIRE)
    QgsVectorLayerCache* cache =
        new QgsVectorLayerCache(layer, 1000, dlg);

    // Modèle attributaire
    QgsAttributeTableModel* model =
        new QgsAttributeTableModel(cache, dlg);
    model->loadLayer();

    //Modèle filtrant (OBLIGATOIRE pour la vue)
QgsMapCanvas* canvas = getCarte()->getCanvas();

QgsAttributeTableFilterModel* filterModel =
    new QgsAttributeTableFilterModel(
        canvas,
        model,
        dlg
    );

    filterModel->setSourceModel(model);
    filterModel->setFilterMode(QgsAttributeTableFilterModel::ShowAll);

    // Vue
    QgsAttributeTableView* view =
        new QgsAttributeTableView(dlg);
    view->setModel(filterModel);

    layout->addWidget(view);
    dlg->setLayout(layout);
    dlg->show();
}


void MainWindow::setProjectActionsEnabled(bool enabled)
{
    // Project-related buttons
    ui->importBtn->setEnabled(enabled);
    ui->transformBtn->setEnabled(enabled);
    ui->addToMapBtn->setEnabled(enabled);
    ui->btnSave->setEnabled(enabled);
    
    // Zoom buttons
    ui->btnZoomPlus->setEnabled(enabled);
    ui->btnZoomMinus->setEnabled(enabled);
    
    // ComboBoxes
    ui->sourceCRSCombo->setEnabled(enabled);
    ui->targetCRSCombo->setEnabled(enabled);
    ui->dimensionCombo->setEnabled(enabled);
    
    // LineEdit
    ui->epochEdit->setEnabled(enabled);
    
    // Layers
    ui->layersList->setEnabled(enabled);
    
    // Map
    ui->carte->setEnabled(enabled);
}

void MainWindow::updateScaleLabel(double scaleValue)
{
    //If there is no project, there is no scale 
    if (currentProject == nullptr) {
        ui->scale->setText("Échelle : N/A");
        return;
    }

    qDebug() << "Scale changed !";
    
    //Round the scale from the double that the QgsMapCanvas::scaleChanged function returns
    int roundedScale = static_cast<int>(std::round(scaleValue / 100) * 100);
    ui->scale->setText(QString("Échelle : 1:%1").arg(roundedScale));
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

Project *MainWindow::getCurrentProject() { return currentProject; }



void MainWindow::setNewProject()
{
    //Reset the canvas
    carte->clearUserLayers();
    //Clear the layers list in the UI
    ui->layersList->clear();
    
    QDialog chosingCRSDialog(this);
    chosingCRSDialog.setWindowTitle("Nouveau projet");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);

    QLabel *dialogText = new QLabel(
        "Choisissez un CRS et une époque pour votre projet",
        &chosingCRSDialog
    );

    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);


    QLineEdit *nameTextZone = new QLineEdit(&chosingCRSDialog);
    nameTextZone->setPlaceholderText("Entrez le nom du projet");


    QComboBox *crsList = new QComboBox(&chosingCRSDialog);
    crsList->setPlaceholderText("Choisissez un CRS");
    setCrsList(crsList);

    QLineEdit *epochTextZone = new QLineEdit(&chosingCRSDialog);
    epochTextZone->setPlaceholderText("Ex : 2025.22");
    epochTextZone->setLocale(QLocale::c());

    QDoubleValidator *epochValidator = new QDoubleValidator(0, 3000, 6, &chosingCRSDialog);
    epochValidator->setNotation(QDoubleValidator::StandardNotation);
    epochTextZone->setValidator(epochValidator);


    QCalendarWidget *calendar = new QCalendarWidget(&chosingCRSDialog);
    // QLabel *decimalDate = new QLabel("Date décimale : ", &chosingCRSDialog);

    // Met à jour epochTextZone quand on clique sur le calendrier
    getCalendarDays(calendar, epochTextZone);

    // Met à jour le calendrier quand on tape une epoch
    connect(epochTextZone, &QLineEdit::editingFinished, this, [=]() {
        bool ok = false;
        double epoch = QLocale::c().toDouble(epochTextZone->text(), &ok);
        if (!ok || epoch <= 0) return;

        int year = static_cast<int>(epoch);
        double frac = epoch - year;

        int daysInYear = QDate::isLeapYear(year) ? 366 : 365;
        int dayOfYear = static_cast<int>(frac * daysInYear);

        QDate date(year, 1, 1);
        date = date.addDays(dayOfYear);

        if (date.isValid())
            calendar->setSelectedDate(date);
    });

 
    connect(acceptationButton, &QPushButton::clicked,
            &chosingCRSDialog, &QDialog::accept);

    connect(crsList, &QComboBox::currentTextChanged,
            this, [this, crsList]() {
            });


    layout->addWidget(dialogText);
    layout->addWidget(nameTextZone);
    layout->addWidget(crsList);
    layout->addWidget(epochTextZone);
    layout->addWidget(calendar);
    layout->addWidget(acceptationButton);

    if (chosingCRSDialog.exec() != QDialog::Accepted)
    {
        qDebug() << "Création de projet annulée.";
        return;
    }

    QString projectName = nameTextZone->text().trimmed();
    QString selectedCrs = crsList->currentText().trimmed();

    bool epochOk = false;
    double epoch = QLocale::c().toDouble(epochTextZone->text(), &epochOk);


    if (projectName.isEmpty())
    {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez saisir un nom de projet.");
        return;
    }

    if (selectedCrs.isEmpty())
    {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner un CRS.");
        return;
    }

    if (!epochOk || epoch <= 0)
    {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez saisir une époque valide.\n"
                             "Exemple : 2025.22");
        return;
    }

    // Extraire EPSG si nécessaire
    QRegExp rx("\\((\\d+)\\)");
    if (rx.indexIn(selectedCrs) != -1)
    {
        selectedCrs = "EPSG:" + rx.cap(1);
    }

    Project *newProject = new Project(
        projectName.toStdString(),
        epoch,
        selectedCrs.toStdString()
    );

    currentProject = newProject;
    setProjectActionsEnabled(true);

    QgsCoordinateReferenceSystem projectCrs(
        QString::fromStdString(currentProject->getCrs())
    );

    if (!projectCrs.isValid())
    {
        QMessageBox::critical(
            this,
            "CRS invalide",
            "Le CRS sélectionné est invalide."
        );
        return;
    }

    QgsProject::instance()->setCrs(projectCrs);
    carte->getCanvas()->setDestinationCrs(projectCrs);

    QgsRectangle defaultExtent;
    //If it is a projected CRS, wit need to use adequate coordinates
    if (selectedCrs == QString("EPSG::9794") || selectedCrs == QString("EPSG::10674")){
        defaultExtent = QgsRectangle(-500000, 5000000, 1200000, 6500000);
    }
    //else lat/lon for geographic CRS
    else{
        defaultExtent = QgsRectangle(-5.0, 41.0, 10.0, 51.0);
    }
    qDebug() << selectedCrs;
    carte->getCanvas()->setExtent(defaultExtent);
    carte->getCanvas()->refresh();

    qDebug() << "Nouveau projet créé :";
    qDebug() << "  Nom :" << QString::fromStdString(currentProject->getName());
    qDebug() << "  CRS :" << QString::fromStdString(currentProject->getCrs());
    qDebug() << "  Époque :" << currentProject->getEpoch0();

    //Updating the display of the project
    projectDisplay->updateDisplayName();
    projectDisplay->updateDisplayCRS();
    projectDisplay->updateDisplayEpoch0();
}


void MainWindow::getCalendarDays(
    QCalendarWidget *calendar,
    QLineEdit *epochEdit
)
{
    auto updateFromCalendar = [=]() {
        QDate date = calendar->selectedDate();

        double dec = computeDate(
            date.day(),
            date.month(),
            date.year()
        );

        epochEdit->setText(
            QLocale::c().toString(dec, 'f', 6)
        );
    };

    // Initialisation
    updateFromCalendar();

    // Connexion calendrier et epoch
    connect(calendar, &QCalendarWidget::selectionChanged,
            this, updateFromCalendar);
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

void MainWindow::setCrsList(QComboBox *comboBox){
    comboBox->clear();
    
    QStandardItemModel *model = new QStandardItemModel(this);
    
    QStandardItem *cat3D = new QStandardItem("3D");
    cat3D->setFlags(cat3D->flags() & ~Qt::ItemIsEnabled); 
    QFont font = cat3D->font();
    font.setBold(true);
    cat3D->setFont(font);
    model->appendRow(cat3D);
    
    QStringList items3D = {
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
    for(const QString &item : items3D){
        model->appendRow(new QStandardItem(item));
    }
    
    // Catégorie 2D
    QStandardItem *cat2D = new QStandardItem("2D");
    cat2D->setFlags(cat2D->flags() & ~Qt::ItemIsEnabled);
    cat2D->setFont(font);
    model->appendRow(cat2D);
    
    QStringList items2D = {
        "ITRF2020 (9989)",
            "ITRF2014 (7912)",
            "ITRF2008 (7911)",
            "ITRF2005 (7910)",
            "ITRF2000 (7909)",
            "ETRF2020 (10570)",
            "ETRF2014 (8403)",
            "ETRF2005 (8399)",
            "ETRF2000 (7931)",
            "RGF93v2b (9783)",
            "RGM23 (10672)",
    };
    for(const QString &item : items2D){
        model->appendRow(new QStandardItem(item));
    }
    
    // Catégorie Projeté
    QStandardItem *catProj = new QStandardItem("Projeté");
    catProj->setFlags(catProj->flags() & ~Qt::ItemIsEnabled);
    catProj->setFont(font);
    model->appendRow(catProj);
    
    QStringList itemsProj = {
       "RGF93v2b (9794)",
       "RGM23 (10674)",
    };
    for(const QString &item : itemsProj){
        model->appendRow(new QStandardItem(item));
    }
    
    comboBox->setModel(model);
    comboBox->setCurrentIndex(1); // Sélectionne le premier item sélectionnable
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
        qDebug() << "Saving project with" << currentProject->getLayers().size() << "layers";
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

    //Reset the map
    carte->clearUserLayers();

    // Call the overloaded version with the filepath
    loadProject(filepath);
    // Enable all actions on the interface 
    setProjectActionsEnabled(true);
}


void MainWindow::loadProject(const QString &filepath)
{
    if (filepath.isEmpty() || !QFile::exists(filepath))
    {
        qWarning() << "Invalid filepath:" << filepath;
        return;
    }

    try
    {
        Project loadedProject = Project::load(filepath.toStdString());

        Project* newProject = new Project(
            loadedProject.getName(),
            loadedProject.getEpoch0(),
            loadedProject.getCrs(),
            {});

        // Delete the older project if already present
        if (currentProject != nullptr)
        {
            delete currentProject;
            qDebug() << "Already existing project";
        }

        // Assigning the newProject
        currentProject = newProject;

        double epoch = currentProject->getEpoch0();
        int year = static_cast<int>(epoch);
        int dayOfYear = static_cast<int>((epoch - year) * 365);
        QDate projectDate = QDate(year, 1, 1).addDays(dayOfYear);

        //Update UI display of project caracteristics
        projectDisplay->updateDisplayName();
        projectDisplay->updateDisplayCRS();
        projectDisplay->updateDisplayEpoch0();

        ui->layersList->clear();

        // --- QGIS canvas ---
        QgsMapCanvas *canvas = getCarte()->getCanvas();
        QString projectCrsString = QString::fromStdString(currentProject->getCrs());
        QgsCoordinateReferenceSystem projectCRS(projectCrsString);
        canvas->setDestinationCrs(projectCRS);

        DataManager &dm = getDataManager();

        const auto &layers = loadedProject.getLayers();
        qDebug() << "Reloading" << layers.size() << "layer(s)...";

        // ----JUST FOR INITIAL TESTS, WE NEED TO FIND A BETTER WAY TO DO THIS
        //If there are no layer, we choose to set the extent of the canvas to France
        if(layers.empty()){
                QgsRectangle defaultExtent;
                //If it is a projected CRS, wit need to use adequate coordinates
                if (projectCrsString == QString("EPSG::9794") || projectCrsString == QString("EPSG::10674")){
                    defaultExtent = QgsRectangle(-500000, 5000000, 1200000, 6500000);
                }
                //else lat/lon for geographic CRS
                else{
                    defaultExtent = QgsRectangle(-5.0, 41.0, 10.0, 51.0);
                }
                qDebug() << projectCrsString;
                carte->getCanvas()->setExtent(defaultExtent);
                carte->getCanvas()->refresh();
        }

        for (const Layer &layer : layers)
        {
            QString layerName = QString::fromStdString(layer.getName());
            QString dataSource = QString::fromStdString(layer.getDataSource());

            if (dataSource.isEmpty() || !QFile::exists(dataSource))
                continue;

            qDebug() << "Reload layer:" << layerName << "from" << dataSource;

            // --- Détection raster / vector ---
            GDALAllRegister();
            GDALDataset *ds = static_cast<GDALDataset *>(
                GDALOpenEx(dataSource.toStdString().c_str(),
                           GDAL_OF_READONLY | GDAL_OF_VECTOR | GDAL_OF_RASTER,
                           nullptr, nullptr, nullptr));

            if (!ds)
                continue;

            bool isRaster = (ds->GetRasterCount() > 0);
            GDALClose(ds);

            // =========================
            // ===== RASTER LAYER ======
            // =========================
            if (isRaster)
            {
                // --- Backend ---
                RasterLayer *raster = dm.loadRaster(dataSource.toStdString());
                if (!raster)
                    continue;

                raster->setDataSource(dataSource.toStdString());
                currentProject->addLayer(*raster);

                // --- QGIS ---
                QgsRasterLayer *qgsLayer =
                    new QgsRasterLayer(
                        dataSource,
                        layerName,
                        "gdal");

                if (!qgsLayer->isValid())
                {
                    delete qgsLayer;
                    continue;
                }

                qgsLayer->setName(layerName);
                QgsProject::instance()->addMapLayer(qgsLayer);

                auto qgsLayers = canvas->layers();
                qgsLayers.prepend(qgsLayer);
                canvas->setLayers(qgsLayers);
                // canvas->setExtent(qgsLayer->extent()); --for now commented because it does not work properly
                canvas->refresh();
            }
            // =========================
            // ===== VECTOR LAYER ======
            // =========================
            else
            {
                // --- Backend ---
                std::vector<VectorLayer *> vLayers =
                    dm.loadVector(dataSource.toStdString());

                for (VectorLayer *vl : vLayers)
                {
                    if (vl->getName() != layer.getName())
                        continue;

                    vl->setDataSource(dataSource.toStdString());
                    currentProject->addLayer(*vl);

                    // --- QGIS ---
                    QgsVectorLayer *qgsLayer =
                        new QgsVectorLayer(
                            dataSource,
                            layerName,
                            "ogr");

                    if (!qgsLayer->isValid())
                    {
                        delete qgsLayer;
                        continue;
                    }

                    qgsLayer->setName(layerName);
                    QgsProject::instance()->addMapLayer(qgsLayer);

                    auto qgsLayers = canvas->layers();
                    qgsLayers.prepend(qgsLayer);
                    canvas->setLayers(qgsLayers);
                    // canvas->setExtent(qgsLayer->extent()); --for now commented because it does not work properly
                    canvas->refresh();
                    break;
                }
            }

            // --- UI ---
            QListWidgetItem *item = new QListWidgetItem(layerName);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            ui->layersList->addItem(item);
        }

        QMessageBox::information(
            this,
            "Success",
            QString("Project '%1' loaded successfully!\nLayers: %2")
                .arg(QString::fromStdString(currentProject->getName()))
                .arg(currentProject->getLayers().size()));

        //Updating the display of the project
        projectDisplay->updateDisplayName();
        projectDisplay->updateDisplayCRS();
        projectDisplay->updateDisplayEpoch0();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(
            this,
            "Error",
            QString("Failed to load project:\n%1").arg(e.what()));
    }


}

