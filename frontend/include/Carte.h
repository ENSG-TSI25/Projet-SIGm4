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
#include "../ui/ui_mainwindow.h"
#include "mainwindow.h"


class MainWindow;

class Carte : public QObject
{
    Q_OBJECT

public:
    Carte(QWidget* containerFrame, MainWindow* mw);  // Prend le QFrame comme parent
    ~Carte();

    QgsMapCanvas* getCanvas() { return canvas; }
    std::string getCarteEpsg();

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
