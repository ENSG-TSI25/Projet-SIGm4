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
#include <core/Layer.hpp>
#include <core/RasterLayer.hpp>
#include <core/Project.hpp>
#include <QDebug>
#include <vector>


LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(""), layerRaster()
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

    QStringList filenameChar = fileName.split(u'/');
    mw->getUi()->selectedFileLabel->setText(
        QString("Fichier sélectionné: %1").arg(filenameChar.last())
    );
    mw->getUi()->selectedFileLabel->setWordWrap(true);

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


void LayerManager::loadRasterLayer() {
    
    RasterLayer* raster = mw->getDataManager().loadRaster(fileName.toStdString());
    if (!raster) return;
    
    QString gpkgUri = QString::fromStdString(raster->getFilePath());
    layerRaster = new QgsRasterLayer(
        gpkgUri, QString::fromStdString(raster->getName()), "gdal");
    
    if (layerRaster->isValid()) {
        qDebug() << "TEST TEST TEST";
        QgsProject::instance()->addMapLayer(layerRaster);
        
        // Récupère tous les layers
        QList<QgsMapLayer*> allLayers = QgsProject::instance()->mapLayers().values();
        
        // Ordre: raster data layers, puis basemaps
        QList<QgsMapLayer*> newOrder;
        for (auto* l : allLayers) {
            if (l->name() != "OSM" && l->name() != "Satellite") {
                newOrder.prepend(l);  // Data layers en premier
            }
        }
        for (auto* l : allLayers) {
            if (l->name() == "OSM" || l->name() == "Satellite") {
                newOrder.append(l);  // Basemaps en dernier 
            }
        }
        
        mw->getCarte()->getCanvas()->setLayers(newOrder);
        
        // Zoom avec transformation CRS
        QgsCoordinateTransform transform(layerRaster->crs(), 
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsProject::instance());
        mw->getCarte()->getCanvas()->setExtent(
            transform.transformBoundingBox(layerRaster->extent()));
        mw->getCarte()->getCanvas()->refresh();
    } else {
        delete layerRaster;
    }
}

//Connect checkbox with the project layers
void LayerManager::displayLayer() {
    int currentIndex = mw->getUi() -> layersList -> row(mw->getUi() -> layersList -> currentItem());
    QListWidgetItem* item = mw->getUi() -> layersList -> item(currentIndex);

    Project* proj = mw -> getCurrentProject();
    std::vector<Layer> layerList = proj -> getLayers();
    Layer test = layerList[currentIndex];

    if (item -> checkState() == Qt::Checked) {
        mw -> getCurrentProject()-> addLayer(test);      
    }
    if (item -> checkState() == Qt::Unchecked) {
        mw -> getCurrentProject()-> rmLayer(test);
    }   
}