#include "../include/LayerManager.h"
#include "../include/Carte.h"
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
#include <QMessageBox>

#include "core/Project.hpp"
#include "core/VectorLayer.hpp"

#include <qgsmapcanvas.h>
#include <QDebug>
#include <core/DataManager.hpp>
#include "core/Project.hpp"
#include "core/VectorLayer.hpp"
#include "core/DataManager.hpp"

LayerManager::LayerManager(MainWindow *mw) : QObject(mw), mw(mw), fileName(" ")
{
}

void LayerManager::listFiles()
{
    mw->getUi()->selectedFileLabel->setText("");
    QWidget *parentWidget = qobject_cast<QWidget *>(parent());

    fileName = QFileDialog::getOpenFileName(
        parentWidget,
        tr("Open window"),
        "$PWD",
        tr("Files (*.gpkg)"));

    if (fileName.isEmpty())
        return;
}

void LayerManager::addFileToWidget()
{
    if (!fileName.isEmpty())
    {
        QStringList filenameChar = fileName.split(u'/');
        QString layerName = filenameChar.last();

        mw->getUi()->selectedFileLabel->setText(
            QString("Fichier sélectionné: %1").arg(filenameChar.last()));
        mw->getUi()->selectedFileLabel->setWordWrap(true);

        // Check if raster or vector
        GDALAllRegister();
        GDALDataset *dataset = (GDALDataset *)GDALOpenEx(
            fileName.toStdString().c_str(),
            GDAL_OF_READONLY | GDAL_OF_RASTER | GDAL_OF_VECTOR,
            nullptr, nullptr, nullptr);

        if (!dataset)
        {
            qWarning() << "Cannot open file:" << fileName;
            return;
        }

        bool isRaster = (dataset->GetRasterCount() > 0);
        GDALClose(dataset);

        // Route to appropriate loader
        if (isRaster)
        {
            loadRasterLayerFromFile(fileName);
            fileName = "";
            return;
        }
        else
        {
            loadVectorLayerFromFile(fileName);
            fileName = "";
            return;
        }

        QListWidgetItem *item = new QListWidgetItem(layerName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        mw->getUi()->layersList->addItem(item);

        // --- Récupérer le projet actuel ---
        Project *proj = mw->getCurrentProject();
        if (!proj)
        {
            qDebug() << "Aucun projet actif. Impossible d'ajouter une couche.";
            return;
        }

        // --- Charger la ou les VectorLayer via DataManager ---
        DataManager dm;
        std::vector<VectorLayer *> layers = dm.loadVector(fileName.toStdString());
        if (layers.empty())
        {
            qDebug() << "Aucun layer chargé depuis :" << fileName;
            return;
        }

        // --- Ajouter chaque layer au projet avec le dataSource ---
        for (auto *l : layers)
        {
            // Définir le dataSource avant d'ajouter
            l->setDataSource(fileName.toStdString());

            proj->addLayer(*l);
            qDebug() << "Couche ajoutée au projet :" << QString::fromStdString(l->getName())
                     << "avec dataSource :" << fileName;

            QString layerName = QString::fromStdString(l->getName());
            QListWidgetItem *item = new QListWidgetItem(layerName);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            mw->getUi()->layersList->addItem(item);
        }

        // --- Récupérer la dernière layer ajoutée ---
        VectorLayer *vLayer = layers.back();
        if (!vLayer)
        {
            qDebug() << "La dernière couche n'est pas une VectorLayer.";
            return;
        }

        QString qlayerName = QString::fromStdString(vLayer->getName());
        QgsVectorLayer *qlayer = new QgsVectorLayer("Point?crs=EPSG:2154", qlayerName, "memory");

        // Ajouter un champ "id"
        QgsFields fields;
        fields.append(QgsField("id", QVariant::Int));
        qlayer->dataProvider()->addAttributes(fields.toList());
        qlayer->updateFields();

        // --- Ajouter les géométries depuis les EWKT ---
        auto ewkts = vLayer->getEWKT();
        int fid = 0;
        for (const auto &ewkt : ewkts)
        {
            if (ewkt.empty())
                continue;

            // Supprimer le préfixe "SRID=xxxx;" si présent
            std::string wkt = ewkt;
            size_t pos = ewkt.find(';');
            if (pos != std::string::npos)
            {
                wkt = ewkt.substr(pos + 1);
            }

            QgsGeometry qgsGeom = QgsGeometry::fromWkt(QString::fromStdString(wkt));
            if (!qgsGeom.isEmpty())
            {
                QgsFeature feat;
                feat.setGeometry(qgsGeom);
                feat.setAttributes({fid++});
                qlayer->dataProvider()->addFeature(feat);
            }
            else
            {
                qDebug() << "[WARN] Géométrie vide pour :" << QString::fromStdString(ewkt);
            }
            qDebug() << "EWKT Feature" << fid << ":" << QString::fromStdString(ewkt);
        }
        qlayer->updateExtents();

        // --- Ajouter au projet QGIS et au canvas ---
        QgsProject::instance()->addMapLayer(qlayer);
        Carte *carte = mw->getCarte();
        QgsMapCanvas *canvas = carte->getCanvas();
        auto currentLayers = canvas->layers();
        currentLayers.prepend(qlayer);
        canvas->setLayers(currentLayers);
        canvas->setExtent(qlayer->extent());
        canvas->refresh();
        qDebug() << "VectorLayer affichée sur le canvas avec succès !";

        // --- Afficher les CRS de toutes les couches ---
        qDebug() << "Liste des CRS des couches sur le canvas :";
        for (auto *layer : currentLayers)
        {
            if (!layer)
                continue;
            QString crs = layer->crs().authid();
            qDebug() << "Layer:" << layer->name() << "CRS:" << crs;
        }

        // --- Afficher l'étendue réelle de la couche ---
        QgsRectangle extent = qlayer->extent();
        qDebug() << "Extent de la couche :"
                 << extent.xMinimum() << extent.yMinimum()
                 << extent.xMaximum() << extent.yMaximum();

        fileName = "";
    }
}

// Duplicate the layer when it's clicked
void LayerManager::duplicateLayer(Dialog *dialog)
{
    QString name = dialog->nameLayer();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    int currentIndex = mw->getUi()->layersList->row(mw->getUi()->layersList->currentItem());
    mw->getUi()->layersList->insertItem(currentIndex, item);
}

// Rename layer selected
void LayerManager::renameLayer(Dialog *dialog)
{
    duplicateLayer(dialog);
    int currentIndex = mw->getUi()->layersList->row(mw->getUi()->layersList->currentItem());
    mw->getUi()->layersList->takeItem(currentIndex);
}

void LayerManager::displayLayerFromFile(const std::string &filepath, const std::string &layerName)
{
    QString qFilepath = QString::fromStdString(filepath);

    // Vérifier que le fichier existe
    if (!QFile::exists(qFilepath))
    {
        qDebug() << "[ERROR] Fichier introuvable:" << qFilepath;
        return;
    }

    qDebug() << "Chargement de la couche depuis:" << qFilepath;

    // Utiliser DataManager pour charger la couche (comme dans addFileToWidget)
    DataManager dm;
    std::vector<VectorLayer *> layers = dm.loadVector(filepath);

    if (layers.empty())
    {
        qDebug() << "[ERROR] Impossible de charger la couche depuis:" << qFilepath;
        return;
    }

    // Prendre la première couche
    VectorLayer *vLayer = layers.front();
    if (!vLayer)
    {
        qDebug() << "[ERROR] La couche chargée est invalide.";
        return;
    }

    QString qlayerName = QString::fromStdString(layerName);
    QgsVectorLayer *qlayer = new QgsVectorLayer("Point?crs=EPSG:2154", qlayerName, "memory");

    // Ajouter un champ "id"
    QgsFields fields;
    fields.append(QgsField("id", QVariant::Int));
    qlayer->dataProvider()->addAttributes(fields.toList());
    qlayer->updateFields();

    // Ajouter les géométries depuis les EWKT
    auto ewkts = vLayer->getEWKT();
    int fid = 0;
    for (const auto &ewkt : ewkts)
    {
        if (ewkt.empty())
            continue;

        // Supprimer le préfixe "SRID=xxxx;" si présent
        std::string wkt = ewkt;
        size_t pos = ewkt.find(';');
        if (pos != std::string::npos)
        {
            wkt = ewkt.substr(pos + 1);
        }

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

    // Ajouter au projet QGIS et au canvas
    QgsProject::instance()->addMapLayer(qlayer);
    Carte *carte = mw->getCarte();
    QgsMapCanvas *canvas = carte->getCanvas();
    auto currentLayers = canvas->layers();
    currentLayers.prepend(qlayer);
    canvas->setLayers(currentLayers);
    canvas->setExtent(qlayer->extent());
    canvas->refresh();

    qDebug() << "Couche affichée sur la carte:" << qlayerName;
}

LayerManager::~LayerManager() {}

void LayerManager::loadRasterLayerFromFile(const QString& file) {
    RasterLayer* raster = mw->getDataManager().loadRaster(file.toStdString());
    if (!raster) {
        qDebug() << "ERROR: loadRaster returned nullptr for" << file;
        return;
    }
    
    QString tableName = QString::fromStdString(raster->getName());
    QString gpkgUri = QString("GPKG:%1:%2").arg(file).arg(tableName);
    
    QgsRasterLayer* layer = new QgsRasterLayer(gpkgUri, tableName, "gdal");
    
    if (!layer->isValid()) {
        qDebug() << "Raster layer error:" << layer->error().message();
        delete layer;
        return;
    }
    
    QgsProject::instance()->addMapLayer(layer);
    
    QListWidgetItem *item = new QListWidgetItem(tableName);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->addItem(item);
    
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
    
    QgsCoordinateReferenceSystem canvasCrs = mw->getCarte()->getCanvas()->mapSettings().destinationCrs();
    if (!canvasCrs.isValid()) {
        mw->getCarte()->getCanvas()->setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    }
    
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




void LayerManager::loadVectorLayerFromFile(const QString &file)
{
    Project *proj = mw->getCurrentProject();
    if (!proj)
    {
        QMessageBox::warning(
            nullptr,
            "Projet manquant",
            "Aucun projet actif. Impossible d'ajouter une couche.");
        return;
    }

    qDebug() << "Loading vector layer from:" << file;

    // Open vector layer with OGR
    QgsVectorLayer *layer = new QgsVectorLayer(file, QFileInfo(file).baseName(), "ogr");

    if (!layer->isValid())
    {
        qWarning() << "Invalid vector layer:" << file;
        delete layer;
        return;
    }

    // Add to project
    QgsProject::instance()->addMapLayer(layer);

    QListWidgetItem *item = new QListWidgetItem(QFileInfo(file).baseName());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    mw->getUi()->layersList->addItem(item);

    // Layer ordering (data layers before basemaps)
    QList<QgsMapLayer *> allLayers = QgsProject::instance()->mapLayers().values();
    QList<QgsMapLayer *> newOrder;

    for (auto *l : allLayers)
    {
        if (l->name() != "OSM" && l->name() != "Satellite")
        {
            newOrder.prepend(l);
        }
    }
    for (auto *l : allLayers)
    {
        if (l->name() == "OSM" || l->name() == "Satellite")
        {
            newOrder.append(l);
        }
    }

    mw->getCarte()->getCanvas()->setLayers(newOrder);

    // Set canvas CRS if not set
    QgsCoordinateReferenceSystem canvasCrs = mw->getCarte()->getCanvas()->mapSettings().destinationCrs();
    if (!canvasCrs.isValid())
    {
        qDebug() << "DEBUG: Canvas CRS not set, setting to EPSG:3857";
        mw->getCarte()->getCanvas()->setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    }

    // Zoom to layer extent with CRS transform
    QgsCoordinateTransform transform(
        layer->crs(),
        QgsCoordinateReferenceSystem("EPSG:3857"),
        QgsProject::instance());

    QgsRectangle transformedExtent = transform.transformBoundingBox(layer->extent());
    qDebug() << "DEBUG: Transformed extent:" << transformedExtent.toString();

    mw->getCarte()->getCanvas()->setExtent(transformedExtent);
    mw->getCarte()->getCanvas()->refresh();
}