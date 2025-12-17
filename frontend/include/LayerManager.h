#pragma once

//Frontend files
#include "../ui/ui_mainwindow.h"
#include "mainwindow.h"
#include "dialogLayerManagement.h"
#include "Carte.h"

//Qt library
#include <QListWidgetItem>
#include <QFileInfo>

#include <core/DataManager.hpp>
#include <core/RasterLayer.hpp>
//QGIS API library
#include <qgsmaptoolpan.h>
#include <qgsrasterlayer.h>
#include <qgsmaplayer.h>
#include <qgslayertree.h>
#include <qgslayertreenode.h>
#include <qgslayertreelayer.h>


class MainWindow;

class LayerManager : public QObject
{
    Q_OBJECT

public:
    LayerManager(MainWindow *mw);
    ~LayerManager();

    void listFiles();
    void addFileToWidget();

    void duplicateLayer(Dialog* dialog);
    void renameLayer(Dialog* dialog);
    
    void displayLayer(); 
    void openDialogFile();

    void loadRasterLayerFromFile(const QString &file);
    void loadVectorLayerFromFile(const QString &file);
    void displayLayerFromFile(const std::string &filepath, const std::string &layerName);

private:
    MainWindow *mw;
    QString fileName;
    void showAddTemporalFieldDialog(const QString &filePath, const std::string &layerName);
    QgsRasterLayer* layerRaster;

};
