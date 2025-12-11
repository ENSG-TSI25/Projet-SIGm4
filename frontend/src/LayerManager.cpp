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
#include <qgsmapcanvas.h>
#include <QDebug>
#include <core/DataManager.hpp>
#include "core/Project.hpp"
#include "core/VectorLayer.hpp"
#include "core/DataManager.hpp"

LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(" ")
{
}


void LayerManager::listFiles() {
    mw->getUi()->selectedFileLabel->setText("");

    QWidget* parentWidget = qobject_cast<QWidget*>(parent());

    fileName = QFileDialog::getOpenFileName(
        parentWidget,
        tr("Open window"),
        "$PWD",
        tr("Files (*.gpkg)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    QStringList filenameChar = fileName.split(u'/');
    mw->getUi()->selectedFileLabel->setText(
        QString("Fichier sélectionné: %1").arg(filenameChar.last())
    );
    mw->getUi()->selectedFileLabel->setWordWrap(true);
    qDebug() << "FileName: " << fileName;

}

void LayerManager::addFileToWidget() { 
    if (!fileName.isEmpty()) {
        QStringList filenameChar = fileName.split(u'/');
        QString layerName = filenameChar.last();
        QListWidgetItem *item = new QListWidgetItem(layerName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);

        mw->getUi()->layersList->addItem(item);



    // --- Récupérer le projet actuel ---
    Project* proj = mw->getCurrentProject();
    if (!proj) {
        qDebug() << "Aucun projet actif. Impossible d'ajouter une couche.";
        return;
    }

    // --- Charger la ou les VectorLayer via DataManager ---
    DataManager dm;
    std::vector<VectorLayer*> layers = dm.loadVector(fileName.toStdString());
    if (layers.empty()) {
        qDebug() << "Aucun layer chargé depuis :" << fileName;
        return;
    }

    // --- Ajouter chaque layer au projet et à la QListWidget ---
    for (auto* l : layers) {
        proj->addLayer(*l);
        qDebug() << "Couche ajoutée au projet :" << QString::fromStdString(l->getName());

        QString layerName = QString::fromStdString(l->getName());
        QListWidgetItem* item = new QListWidgetItem(layerName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        mw->getUi()->layersList->addItem(item);
    }

    // --- Récupérer la dernière layer ajoutée ---
    VectorLayer* vLayer = layers.back();
    if (!vLayer) {
        qDebug() << "La dernière couche n'est pas une VectorLayer.";
        return;
    }

    QString qlayerName = QString::fromStdString(vLayer->getName());
    QgsVectorLayer* qlayer = new QgsVectorLayer("Point?crs=EPSG:2154", qlayerName, "memory");

    // Ajouter un champ "id"
    QgsFields fields;
    fields.append(QgsField("id", QVariant::Int));
    qlayer->dataProvider()->addAttributes(fields.toList());
    qlayer->updateFields();

    // --- Ajouter les géométries depuis les EWKT ---
    auto ewkts = vLayer->getEWKT();
    int fid = 0;
    for (const auto& ewkt : ewkts) {
        if (ewkt.empty()) continue;

        // Supprimer le préfixe "SRID=xxxx;" si présent
        std::string wkt = ewkt;
        size_t pos = ewkt.find(';');
        if (pos != std::string::npos) {
            wkt = ewkt.substr(pos + 1);
        }

        QgsGeometry qgsGeom = QgsGeometry::fromWkt(QString::fromStdString(wkt));
        if (!qgsGeom.isEmpty()) {
            QgsFeature feat;
            feat.setGeometry(qgsGeom);
            feat.setAttributes({fid++});
            qlayer->dataProvider()->addFeature(feat);
        } else {
            qDebug() << "[WARN] Géométrie vide pour :" << QString::fromStdString(ewkt);
        }

        qDebug() << "EWKT Feature" << fid << ":" << QString::fromStdString(ewkt);
    }

    qlayer->updateExtents();

    // --- Ajouter au projet QGIS et au canvas ---
    QgsProject::instance()->addMapLayer(qlayer);
    Carte* carte = mw->getCarte();
    QgsMapCanvas* canvas = carte->getCanvas();

    auto currentLayers = canvas->layers();
    currentLayers.prepend(qlayer);  // mettre la nouvelle couche en haut
    canvas->setLayers(currentLayers);
    canvas->setExtent(qlayer->extent()); // zoom automatique
    canvas->refresh();

    qDebug() << "VectorLayer affichée sur le canvas avec succès !";

    // --- Afficher les CRS de toutes les couches ---
    qDebug() << "Liste des CRS des couches sur le canvas :";
    for (auto* layer : currentLayers) {
        if (!layer) continue;
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
