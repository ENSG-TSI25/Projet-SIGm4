#include "../include/LayerManager.h"
#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>
#include <QWidget>

#include <core/DataManager.hpp>
#include <core/RasterLayer.hpp>
#include <QDebug>
#include <gdal_priv.h>
#include <qgsvectorlayer.h>


LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(" ")
{
}



void LayerManager::listFiles(){
    mw->getUi()->selectedFileLabel->setText("");
    QWidget* parentWidget = qobject_cast<QWidget*>(parent());
    
    fileName = QFileDialog::getOpenFileName(
        parentWidget,
        tr("Open window"),
        "$PWD",
        tr("Files (*.gpkg)")
    );
    
    if (fileName.isEmpty()) return;
    
    // Display selected file
    QStringList filenameChar = fileName.split(u'/');
    mw->getUi()->selectedFileLabel->setText(
        QString("Fichier sélectionné: %1").arg(filenameChar.last())
    );
    mw->getUi()->selectedFileLabel->setWordWrap(true);
    
    // Check if raster or vector
    GDALAllRegister();
    GDALDataset* dataset = (GDALDataset*)GDALOpenEx(
        fileName.toStdString().c_str(),
        GDAL_OF_READONLY | GDAL_OF_RASTER | GDAL_OF_VECTOR,
        nullptr, nullptr, nullptr
    );
    
    if (!dataset) {
        qWarning() << "Cannot open file:" << fileName;
        return;
    }
    
    bool isRaster = (dataset->GetRasterCount() > 0);
    GDALClose(dataset);
    
    // Route to appropriate loader
    if (isRaster) {
        loadRasterLayerFromFile(fileName);
    } else {
        loadVectorLayerFromFile(fileName);
    }
}

void LayerManager::addFileToWidget() { 
    if (!fileName.isEmpty()) {
        QStringList filenameChar = fileName.split(u'/');
        QString layerName = filenameChar.last();
        QListWidgetItem *item = new QListWidgetItem(layerName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);

        mw->getUi()->layersList->addItem(item);

        fileName = "";
    }

}


//Duplicate the layer when it's clicked
void LayerManager::duplicateLayer(Dialog* dialog) {
    QString name = dialog-> nameLayer();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    int currentIndex = mw->getUi() -> layersList -> row(mw->getUi() -> layersList -> currentItem());
    mw->getUi() -> layersList -> insertItem(currentIndex, item);
}

//Rename layer selected
void LayerManager::renameLayer(Dialog* dialog) {
    duplicateLayer(dialog);
    int currentIndex = mw->getUi() -> layersList -> row(mw->getUi() -> layersList -> currentItem());
    mw->getUi() -> layersList -> takeItem(currentIndex);
}


LayerManager::~LayerManager() {}


void LayerManager::loadRasterLayerFromFile(const QString& file) {
    RasterLayer* raster = mw->getDataManager().loadRaster(file.toStdString());
    if (!raster) return;
    
    QString gpkgUri = QString::fromStdString(raster->getFilePath());
    QgsRasterLayer* layer = new QgsRasterLayer(
        gpkgUri, QString::fromStdString(raster->getName()), "gdal"
    );
    
    if (layer->isValid()) {
        QgsProject::instance()->addMapLayer(layer);
        
        QList<QgsMapLayer*> allLayers = QgsProject::instance()->mapLayers().values();
        QList<QgsMapLayer*> newOrder;
        
        for (auto* l : allLayers) {
            if (l->name() != "OSM" && l->name() != "Satellite") {
                newOrder.prepend(l);
            }
        }
        for (auto* l : allLayers) {
            if (l->name() == "OSM" || l->name() == "Satellite") {
                newOrder.append(l);
            }
        }
        
        mw->getCarte()->getCanvas()->setLayers(newOrder);
        
        QgsCoordinateTransform transform(
            layer->crs(), 
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsProject::instance()
        );
        mw->getCarte()->getCanvas()->setExtent(
            transform.transformBoundingBox(layer->extent())
        );
        mw->getCarte()->getCanvas()->refresh();
    } else {
        delete layer;
    }
}

void LayerManager::loadVectorLayerFromFile(const QString& file) {
    qDebug() << "Loading vector layer from:" << file;
    
    // Open vector layer with OGR
    QgsVectorLayer* layer = new QgsVectorLayer(file, QFileInfo(file).baseName(), "ogr");
    
    if (!layer->isValid()) {
        qWarning() << "Invalid vector layer:" << file;
        delete layer;
        return;
    }
    
    // Add to project
    QgsProject::instance()->addMapLayer(layer);
    
    // Layer ordering (data layers before basemaps)
    QList<QgsMapLayer*> allLayers = QgsProject::instance()->mapLayers().values();
    QList<QgsMapLayer*> newOrder;
    
    for (auto* l : allLayers) {
        if (l->name() != "OSM" && l->name() != "Satellite") {
            newOrder.prepend(l);
        }
    }
    for (auto* l : allLayers) {
        if (l->name() == "OSM" || l->name() == "Satellite") {
            newOrder.append(l);
        }
    }
    
    mw->getCarte()->getCanvas()->setLayers(newOrder);
    
    // Zoom to layer extent with CRS transform
    QgsCoordinateTransform transform(
        layer->crs(), 
        QgsCoordinateReferenceSystem("EPSG:3857"),
        QgsProject::instance()
    );
    
    mw->getCarte()->getCanvas()->setExtent(
        transform.transformBoundingBox(layer->extent())
    );
    mw->getCarte()->getCanvas()->refresh();
}