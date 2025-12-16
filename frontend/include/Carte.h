#pragma once


//Standard libraries
#include <cmath>

//Frontend files
#include "../ui/ui_mainwindow.h"
#include "mainwindow.h"

//Qt library
#include <QHBoxLayout>

//QGIS API library
#include <qgsmaptoolpan.h>
#include <qgsrasterlayer.h>
#include <qgsmaplayer.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsmaptoolpan.h>


class MainWindow;

class Carte : public QObject
{
    Q_OBJECT

public:
    Carte(QWidget* containerFrame, MainWindow* mw);  // Prend le QFrame comme parent
    ~Carte();

    QgsMapCanvas* getCanvas() { return canvas; }
    std::string getCarteEpsg() { return carteEpsg; }

private:
    MainWindow* mw;
    // Canvas
    QgsMapCanvas* canvas;

    // Layers
    QgsRasterLayer* osmLayer;
    QgsRasterLayer* satLayer;
    bool osmVisible;

    // UI (facultatif si tu veux mettre les boutons ailleurs)

    QPushButton* toggleBasemap;
    std::string carteEpsg;

    QVBoxLayout* layout;

    // Méthodes
    void initCanvas(QWidget* containerFrame);
    void initLayers();
    void connectSignals();
    void updateLabels();

    void toggleBaseLayer();
};
