#include "../include/LayerManager.h"
#include <QSet>

LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(""), layerRaster()
{
}

LayerManager::~LayerManager() {}

// FILE SELECTION

void LayerManager::listFiles()
{
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());

    fileName = QFileDialog::getOpenFileName(
        parentWidget,
        tr("Open file"),
        QDir::currentPath(),
        tr("Geo files (*.gpkg *.shp *.tif *.tiff)")
    );

    if (!fileName.isEmpty()) {
        openDialogFile();
    }

    mw->getUi()->selectedFileLabel->setText(fileName);
}

void LayerManager::openDialogFile() {
    qDebug() << "TEST dialog";
    //Open a new dialog to show all information    
    QDialog infoLayerDialog;
    infoLayerDialog.setWindowTitle("Informations sur le fichier sélectionné");
    infoLayerDialog.show();

    Project* project = mw -> getCurrentProject();
    std::string CRSProject = project -> getCrs();
    double EpochProject = project -> getEpoch0();

    // QStringList filenameChar = fileName.split(u'/');
    // QString *dialogFileText = new QString("Fichier sélectionné: %1");
    // dialogFileText->arg(filenameChar.last());


    // QLabel *dialogTextCRSProject = new QLabel("CRS du projet :", &infoLayerDialog);
    // QLabel *dialogTextCRSFile = new QLabel("CRS du fichier sélectionné :", &infoLayerDialog);
    // QLabel *dialogTextDateProject = new QLabel("Date du projet :", &infoLayerDialog);
    // QLabel *dialogTextDateFile = new QLabel("Date du fichier sélectionné :", &infoLayerDialog);
    // QPushButton *acceptationButton = new QPushButton("OK", &infoLayerDialog);

}

// ENTRY POINT

void LayerManager::addFileToWidget()
{
    if (fileName.isEmpty())
        return;

    // Detect raster / vector using GDAL
    GDALAllRegister();
    GDALDataset* dataset = static_cast<GDALDataset*>(
        GDALOpenEx(
            fileName.toStdString().c_str(),
            GDAL_OF_READONLY | GDAL_OF_RASTER | GDAL_OF_VECTOR,
            nullptr, nullptr, nullptr
        )
    );

    if (!dataset) {
        qWarning() << "Impossible d'ouvrir le fichier :" << fileName;
        return;
    }

    bool isRaster = dataset->GetRasterCount() > 0;
    GDALClose(dataset);

    if (isRaster) {
        loadRasterLayerFromFile(fileName);
    } else {
        loadVectorLayerFromFile(fileName);
    }

    fileName.clear();
}

// VECTOR

void LayerManager::loadVectorLayerFromFile(const QString& file)
{
    Project* proj = mw->getCurrentProject();
    if (!proj)
    {
        QMessageBox::warning(
            nullptr,
            "Projet manquant",
            "Aucun projet actif. Impossible d'ajouter une couche.");
        return;
    }

    QgsMapCanvas* canvas = mw->getCarte()->getCanvas();

    // --- Set canvas CRS if not set (Project CRS) ---
    QgsCoordinateReferenceSystem canvasCrs =
        canvas->mapSettings().destinationCrs();

    if (!canvasCrs.isValid())
    {
        QString projectCrs = QString::fromStdString(proj->getCrs());
        qDebug() << "DEBUG: Canvas CRS not set, setting to" << projectCrs;
        canvas->setDestinationCrs(QgsCoordinateReferenceSystem(projectCrs));
    }

    qDebug() << "Loading vector layer from:" << file;

    // --- Load vector layer ---
    QgsVectorLayer* qgsLayer =
        new QgsVectorLayer(file, QFileInfo(file).baseName(), "ogr");

    if (!qgsLayer->isValid())
    {
        qWarning() << "Invalid vector layer:" << file;
        delete qgsLayer;
        return;
    }


    // --- Charger la ou les VectorLayer via DataManager EXISTANT ---
    DataManager& dm = mw->getDataManager();

    std::vector<VectorLayer*> Vlayers =
        dm.loadVector(fileName.toStdString());

    if (Vlayers.empty())
    {
        qDebug() << "Aucun layer chargé depuis :" << fileName;
        return;
    }

    // --- Ajouter chaque layer au projet ---
    for (VectorLayer* l : Vlayers)
    {
        if (!l) continue;

        l->setDataSource(fileName.toStdString());

        proj->addLayer(*l);

        qDebug() << "Couche ajoutée au projet :"
                << QString::fromStdString(l->getName())
                << "dataSource :" << fileName;

        QString layerName =
        QString::fromStdString(l->getName());

        qgsLayer->setName(layerName);


    }


    // --- Add to QGIS project ---
    QgsProject::instance()->addMapLayer(qgsLayer);

    // --- Add layer on top & zoom on it ---
    QList<QgsMapLayer*> layers = canvas->layers();
    layers.prepend(qgsLayer);
    canvas->setLayers(layers);
    // canvas->setExtent(qgsLayer->extent());
    canvas->refresh();

    // --- UI ---
    QListWidgetItem* item =
        new QListWidgetItem(qgsLayer->name());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->addItem(item);



    qDebug() << "===== Couches du projet =====";

    const auto& projectLayers = proj->getLayers();

    if (projectLayers.empty())
    {
        qDebug() << "Aucune couche dans le projet.";
    }
    else
    {
        for (const auto& l : projectLayers)
        {
            qDebug() << " -"
                    << QString::fromStdString(l.getName());
        }
    }

}


// RASTER
void LayerManager::loadRasterLayerFromFile(const QString& file)
{
    Project* proj = mw->getCurrentProject();
    if (!proj)
    {
        QMessageBox::warning(
            nullptr,
            "Projet manquant",
            "Aucun projet actif. Impossible d'ajouter une couche.");
        return;
    }

    QgsMapCanvas* canvas = mw->getCarte()->getCanvas();

    // --- Set canvas CRS if not set (Project CRS) ---
    QgsCoordinateReferenceSystem canvasCrs =
        canvas->mapSettings().destinationCrs();

    if (!canvasCrs.isValid())
    {
        QString projectCrs = QString::fromStdString(proj->getCrs());
        qDebug() << "DEBUG: Canvas CRS not set, setting to" << projectCrs;
        canvas->setDestinationCrs(QgsCoordinateReferenceSystem(projectCrs));
    }

    qDebug() << "Loading raster layer from:" << file;

    // --- Load raster (backend) ---
    RasterLayer* raster =
        mw->getDataManager().loadRaster(file.toStdString());

    if (!raster)
    {
        qDebug() << "ERROR: loadRaster returned nullptr for" << file;
        return;
    }

    // --- Load raster in QGIS ---
    QgsRasterLayer* qgsLayer =
        new QgsRasterLayer(file,
                           QString::fromStdString(raster->getName()),
                           "gdal");

    if (!qgsLayer->isValid())
    {
        qDebug() << "Raster layer error:" << qgsLayer->error().message();
        delete qgsLayer;
        return;
    }

    QgsProject::instance()->addMapLayer(qgsLayer);

    // --- Add layer on top & zoom on it ---
    QList<QgsMapLayer*> layers = canvas->layers();
    layers.prepend(qgsLayer);
    canvas->setLayers(layers);
    // canvas->setExtent(qgsLayer->extent());
    canvas->refresh();



    // --- Charger le RasterLayer via DataManager EXISTANT ---
    DataManager& dm = mw->getDataManager();

    RasterLayer* rlayer =
        dm.loadRaster(fileName.toStdString());

    if (!rlayer)
    {
        qDebug() << "Aucun raster chargé depuis :" << fileName;
        return;
    }

    // --- Ajouter le raster au projet ---
    rlayer->setDataSource(fileName.toStdString());

    proj->addLayer(*rlayer);

    QString layerName =
        QString::fromStdString(rlayer->getName());

    qgsLayer->setName(layerName);

    qDebug() << "Raster ajouté au projet :"
            << QString::fromStdString(rlayer->getName())
            << "dataSource :" << fileName;

    // --- UI ---
    QListWidgetItem* item =
        new QListWidgetItem(qgsLayer->name());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->addItem(item);




    qDebug() << "===== Couches du projet =====";

    const auto& projectLayers = proj->getLayers();

    if (projectLayers.empty())
    {
        qDebug() << "Aucune couche dans le projet.";
    }
    else
    {
        for (const auto& l : projectLayers)
        {
            qDebug() << " -"
                    << QString::fromStdString(l.getName());
        }
    }

}

#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>

// UI HELPERS

bool LayerManager::isLayerNameUnique(const QString& name)
{
    QListWidget* listWidget = mw->getUi()->layersList;
    for(int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == name) {
            return false;
        }
    }
    return true;
}

QColor LayerManager::getLayerColor()
{
    int index = mw->getUi()->layersList->currentRow();
    if (index < 0) return QColor();

    QListWidgetItem* item = mw->getUi()->layersList->item(index);
    QString layerName = item->text();

    QList<QgsMapLayer*> foundLayers = QgsProject::instance()->mapLayersByName(layerName);
    if (foundLayers.isEmpty()) return QColor();

    QgsMapLayer* layer = foundLayers.first();
    QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>(layer);
    if (vLayer) {
        if (vLayer->renderer()) {
            QgsRenderContext context;
            QgsSymbolList symbols = vLayer->renderer()->symbols(context);
            if (!symbols.isEmpty()) {
                return symbols.first()->color();
            }
        }
    }
    
    return QColor();
}

void LayerManager::changeLayerColor(const QColor& color)
{
    int index = mw->getUi()->layersList->currentRow();
    if (index < 0) return;

    QListWidgetItem* item = mw->getUi()->layersList->item(index);
    QString layerName = item->text();

    QList<QgsMapLayer*> foundLayers = QgsProject::instance()->mapLayersByName(layerName);
    if (foundLayers.isEmpty()) return;

    QgsMapLayer* layer = foundLayers.first();
    QgsVectorLayer* vLayer = qobject_cast<QgsVectorLayer*>(layer);
    if (vLayer) {
        if (vLayer->renderer()) {
             QgsRenderContext context;
             QgsSymbolList symbols = vLayer->renderer()->symbols(context);
             if (!symbols.isEmpty()) {
                 QgsSymbol* symbol = symbols.first();
                 if (symbol) {
                     symbol->setColor(color);
                     vLayer->triggerRepaint();
                     mw->getCarte()->getCanvas()->refresh();
                 }
             }
        }
    }
}

void LayerManager::duplicateLayer(Dialog* dialog)
{
    int currentIndex = mw->getUi()->layersList->currentRow();
    if (currentIndex < 0) return;

    QString newLayerName = dialog->nameLayer();
    
    if (newLayerName.isEmpty()) {
        QMessageBox::warning(mw, "Erreur", "Le nom de la couche ne peut pas être vide.");
        return;
    }

    if (!isLayerNameUnique(newLayerName)) {
        QMessageBox::warning(mw, "Erreur", "Une couche avec ce nom existe déjà.");
        return;
    }
    
    QgsMapCanvas* canvas = mw->getCarte()->getCanvas();
    
    QListWidgetItem* currentItem = mw->getUi()->layersList->item(currentIndex);
    QString sourceLayerName = currentItem->text();
    
    QList<QgsMapLayer*> foundLayers = QgsProject::instance()->mapLayersByName(sourceLayerName);
    if (foundLayers.isEmpty()) return;
    QgsMapLayer* sourceLayer = foundLayers.first();

  
    QgsMapLayer* newLayer = sourceLayer->clone();
    newLayer->setName(newLayerName);

    QgsProject::instance()->addMapLayer(newLayer, false);


    QListWidgetItem* item = new QListWidgetItem(newLayerName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->insertItem(currentIndex, item);

   
    displayLayer();
}


void LayerManager::renameLayer(Dialog* dialog)
{
    int index = mw->getUi()->layersList->currentRow();
    if (index < 0) return;

    QString newName = dialog->nameLayer();
    
    if (newName.isEmpty()) {
        QMessageBox::warning(mw, "Erreur", "Le nom de la couche ne peut pas être vide.");
        return;
    }

    if (!isLayerNameUnique(newName)) {
        QMessageBox::warning(mw, "Erreur", "Une couche avec ce nom existe déjà.");
        return;
    }

    QListWidgetItem* item = mw->getUi()->layersList->item(index);
    QString oldName = item->text();
    
   
    item->setText(newName);
    
    
    QList<QgsMapLayer*> foundLayers = QgsProject::instance()->mapLayersByName(oldName);
    if (!foundLayers.isEmpty()) {
        foundLayers.first()->setName(newName);
    }
    
    
    displayLayer();
}

void LayerManager::deleteLayer()
{
    int index = mw->getUi()->layersList->currentRow();
    if (index < 0) return;

    QListWidgetItem* item = mw->getUi()->layersList->item(index);
    QString layerName = item->text();

    
    delete mw->getUi()->layersList->takeItem(index);

    QList<QgsMapLayer*> foundLayers = QgsProject::instance()->mapLayersByName(layerName);
    if (!foundLayers.isEmpty()) {
        QgsProject::instance()->removeMapLayer(foundLayers.first());
    }

    
    displayLayer();
}



void LayerManager::displayLayer() {
    QgsMapCanvas* canvas = mw->getCarte()->getCanvas();
    QListWidget* listWidget = mw->getUi()->layersList;

    
    QList<QgsMapLayer*> baseLayers;
    QList<QgsMapLayer*> currentCanvasLayers = canvas->layers();

 
    QSet<QString> managedLayerNames;
    for(int i = 0; i < listWidget->count(); ++i) {
        managedLayerNames.insert(listWidget->item(i)->text());
    }

    for(QgsMapLayer* layer : currentCanvasLayers) {
        if(!managedLayerNames.contains(layer->name())) {
            baseLayers.append(layer);
        }
    }

 
    QList<QgsMapLayer*> newLayers = baseLayers;

  
    for(int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        if(item->checkState() == Qt::Checked) {
            QString layerName = item->text();
            
            QList<QgsMapLayer*> foundLayers = QgsProject::instance()->mapLayersByName(layerName);
            if(!foundLayers.isEmpty()) {
                newLayers.prepend(foundLayers.first());
            }
        }
    }

    canvas->setLayers(newLayers);
    canvas->refresh();
}
