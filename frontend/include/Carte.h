#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsproject.h>
#include <qgsmaptoolpan.h>

#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include <qgsmapcanvas.h>




class Carte : public QObject
{
    Q_OBJECT

public:
    Carte(QWidget* containerFrame);  // Prend le QFrame comme parent
    ~Carte();

    QgsMapCanvas* getCanvas() { return canvas; }

private:
    // Canvas
    QgsMapCanvas* canvas;

    // Layers
    QgsRasterLayer* osmLayer;
    QgsRasterLayer* satLayer;

    bool osmVisible;

    // UI (facultatif si tu veux mettre les boutons ailleurs)

    QPushButton* toggleBasemap;

    QVBoxLayout* layout;

    // Méthodes
    void initCanvas(QWidget* containerFrame);
    void initLayers();
    void connectSignals();
    void updateLabels();

    void toggleBaseLayer();
    QgsPointXY wgs84ToMercator(double lon, double lat);
};
