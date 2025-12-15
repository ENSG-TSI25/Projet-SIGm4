#pragma once

//Frontend files
#include "../ui/ui_mainwindow.h"
#include "mainwindow.h"
#include "dialogLayerManagement.h"

//QGIS API library
#include <qgsmaptoolpan.h>
#include <qgsrasterlayer.h>
#include <qgsmaplayer.h>


class MainWindow;

class LayerManager : public QObject
{
    Q_OBJECT

public:
    LayerManager(MainWindow* mw);
    ~LayerManager();

    void listFiles();
    void addFileToWidget();

    void duplicateLayer(Dialog* dialog);
    void renameLayer(Dialog* dialog);
    
    void loadRasterLayerFromFile(const QString& file);
    void loadVectorLayerFromFile(const QString& file);
    void displayLayerFromFile(const std::string& filepath, const std::string& layerName);


private:
    MainWindow* mw;
    QString fileName;

};
