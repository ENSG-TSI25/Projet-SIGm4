#include "../include/LayerManager.h"
#include "../include/Carte.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsproject.h>
#include <qgscoordinatereferencesystem.h>
#include <gdal_priv.h>
#include <core/DataManager.hpp>
#include <core/Project.hpp>
#include <core/VectorLayer.hpp>
#include <core/RasterLayer.hpp>
#include <core/GeoPackageReader.hpp>
#include <QSet>

LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(""), layerRaster()
{
}

LayerManager::~LayerManager() {}

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
    if (fileName.isEmpty()) return;

    GDALAllRegister();
    GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_READONLY | GDAL_OF_RASTER | GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    if (!dataset) return;

    bool isRaster = dataset->GetRasterCount() > 0;
    GDALClose(dataset);

    if (isRaster) {
        loadRasterLayerFromFile(fileName);
    } else {
        loadVectorLayerFromFile(fileName);
    }
    fileName.clear();
}

void LayerManager::loadVectorLayerFromFile(const QString& file)
{
    Project* proj = mw->getCurrentProject();
    if (!proj) {
        QMessageBox::warning(nullptr, "Projet manquant", "Aucun projet actif.");
        return;
    }

    QgsMapCanvas* canvas = mw->getCarte()->getCanvas();
    QgsCoordinateReferenceSystem canvasCrs = canvas->mapSettings().destinationCrs();
    if (!canvasCrs.isValid()) {
        QString projectCrs = QString::fromStdString(proj->getCrs());
        canvas->setDestinationCrs(QgsCoordinateReferenceSystem(projectCrs));
    }

    QgsVectorLayer* qgsLayer = new QgsVectorLayer(file, QFileInfo(file).baseName(), "ogr");
    if (!qgsLayer->isValid()) {
        delete qgsLayer;
        return;
    }

    QgsProject::instance()->addMapLayer(qgsLayer);

    DataManager dm;
    std::vector<VectorLayer*> backendLayers = dm.loadVector(file.toStdString());
    for (auto* l : backendLayers) {
        l->setDataSource(file.toStdString());
        auto vectorLayer = std::make_shared<VectorLayer>(*l);
        proj->addLayer(vectorLayer);

        if (!l->hasTemporalData()) {
            QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "No Temporal Field Detected",
                QString("Layer '%1' has no temporal field.\nAdd one?").arg(QString::fromStdString(l->getName())),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                showAddTemporalFieldDialog(file, l->getName());
            }
        } else {
            qDebug() << "Layer has temporal field:" << QString::fromStdString(l->getTimestampField());
        }
    }

    QListWidgetItem* item = new QListWidgetItem(qgsLayer->name());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->addItem(item);

    // --- Add to QGIS project ---
    QgsProject::instance()->addMapLayer(qgsLayer);
    canvas->setCurrentLayer(qgsLayer);

    // --- UI list item ---
    QListWidgetItem* item = new QListWidgetItem(qgsLayer->name());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    // link UI <-> layers QGIS
    item->setData(Qt::UserRole, qgsLayer->id());

    mw->getUi()->layersList->addItem(item);

    // --- Add layer on top & zoom ---
    QList<QgsMapLayer*> layers = canvas->layers();
    layers.prepend(qgsLayer);
    canvas->setLayers(layers);
    // canvas->setExtent(qgsLayer->extent());
    canvas->refresh();



    // // --- UI ---
    // QListWidgetItem* item =
    //     new QListWidgetItem(qgsLayer->name());
    // item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    // item->setCheckState(Qt::Checked);
    // mw->getUi()->layersList->addItem(item);



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

void LayerManager::loadRasterLayerFromFile(const QString& file)
{
    Project* proj = mw->getCurrentProject();
    if (!proj) {
        QMessageBox::warning(nullptr, "Projet manquant", "Aucun projet actif.");
        return;
    }

    RasterLayer* raster = mw->getDataManager().loadRaster(file.toStdString());
    if (!raster) return;

    QString gpkgUri = QString("GPKG:%1:%2").arg(file).arg(QString::fromStdString(raster->getName()));
    QgsRasterLayer* qgsLayer = new QgsRasterLayer(gpkgUri, QString::fromStdString(raster->getName()), "gdal");
    if (!qgsLayer->isValid()) {
        delete qgsLayer;
        return;
    }

    QgsProject::instance()->addMapLayer(qgsLayer);
    canvas->setCurrentLayer(qgsLayer);


    raster->setDataSource(file.toStdString());
    auto rasterLayer = std::make_shared<RasterLayer>(*raster);
    proj->addLayer(rasterLayer);
    // --- Add layer on top & zoom on it ---
    QList<QgsMapLayer*> layers = canvas->layers();
    layers.prepend(qgsLayer);
    canvas->setLayers(layers);
    // canvas->setExtent(qgsLayer->extent());
    canvas->refresh();

    QListWidgetItem* item = new QListWidgetItem(qgsLayer->name());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->addItem(item);

    QgsMapCanvas* canvas = mw->getCarte()->getCanvas();
    QList<QgsMapLayer*> layers = canvas->layers();
    layers.prepend(qgsLayer);
    canvas->setLayers(layers);
    canvas->setExtent(qgsLayer->extent());
    canvas->refresh();
}

void LayerManager::showAddTemporalFieldDialog(const QString& filePath, const std::string& layerName)
{
    QDialog dialog;
    dialog.setWindowTitle("Add Temporal Field");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    layout->addWidget(new QLabel("Field name:"));
    QLineEdit* fieldInput = new QLineEdit("t");
    layout->addWidget(fieldInput);
    
    layout->addWidget(new QLabel("Default epoch value:"));
    QLineEdit* epochInput = new QLineEdit("2025.0");
    layout->addWidget(epochInput);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() == QDialog::Accepted) {
        QString fieldName = fieldInput->text();
        double epochValue = epochInput->text().toDouble();
        
        GeoPackageReader reader(filePath.toStdString());
        if (reader.open()) {
            bool success = reader.addTemporalField(layerName, fieldName.toStdString(), epochValue);
            reader.close();
            
            if (success) {
                QMessageBox::information(nullptr, "Success", QString("Temporal field '%1' added successfully").arg(fieldName));
                loadVectorLayerFromFile(filePath);
            } else {
                QMessageBox::critical(nullptr, "Error", "Failed to add temporal field");
            }
        }
    }
}

void LayerManager::duplicateLayer(Dialog* dialog)
{
    QString name = dialog->nameLayer();
    QListWidgetItem* item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    int index = mw->getUi()->layersList->currentRow();
    mw->getUi()->layersList->insertItem(index, item);
}

void LayerManager::renameLayer(Dialog* dialog)
{
    int index = mw->getUi()->layersList->currentRow();
    if (index < 0) return;
    mw->getUi()->layersList->item(index)->setText(dialog->nameLayer());
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
