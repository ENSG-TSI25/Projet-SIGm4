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

        


private:
    MainWindow* mw;
    QString fileName;

};
