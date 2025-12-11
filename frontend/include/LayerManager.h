#pragma once
#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsproject.h>
#include <qgsmaptoolpan.h>

#include <qgsrasterlayer.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include "../ui/ui_mainwindow.h"
#include "mainwindow.h"
#include <QDialog>
#include "dialogLayerManagement.h"

#include <core/DataManager.hpp>
#include <core/RasterLayer.hpp>



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


private:
    MainWindow* mw;
    QString fileName;

};
