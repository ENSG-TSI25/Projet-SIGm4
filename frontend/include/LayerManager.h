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

/**
 * @file LayerManager.h
 * @brief Layer management system
 * 
 * @class LayerManager
 * @brief Manages all layers in the application
 * 
 * Handles file loading, layer listing, duplication,
 * and renaming operations.
 */
class LayerManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param mw Pointer to main window
     */
    LayerManager(MainWindow* mw);
    
    /**
     * @brief Destructor
     */
    ~LayerManager();

    /**
     * @brief Lists available layer files
     */
    void listFiles();
    
    /**
     * @brief Adds selected file to layer widget
     */
    void addFileToWidget();

    /**
     * @brief Duplicates a layer
     * @param dialog Layer management dialog
     */
    void duplicateLayer(Dialog* dialog);
    
    /**
     * @brief Renames a layer
     * @param dialog Layer management dialog
     */
    void renameLayer(Dialog* dialog);

private:
    MainWindow* mw;     ///< Main window reference
    QString fileName;   ///< Current file name
};