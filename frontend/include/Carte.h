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

/**
 * @file Carte.h
 * @brief Map display widget
 * 
 * @class Carte
 * @brief Interactive map canvas for displaying geospatial data
 * 
 * Qt widget that shows OpenStreetMap or satellite basemaps
 * and vector layers using QGIS libraries.
 */
class Carte : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param containerFrame Parent widget for the map canvas
     */
    Carte(QWidget* containerFrame);
    
    /**
     * @brief Destructor
     */
    ~Carte();

    /**
     * @brief Gets the map canvas
     * @return Pointer to QgsMapCanvas
     */
    QgsMapCanvas* getCanvas() { return canvas; }

private:
    QgsMapCanvas* canvas;          ///< Map canvas widget
    QgsRasterLayer* osmLayer;      ///< OpenStreetMap layer
    QgsRasterLayer* satLayer;      ///< Satellite imagery layer
    bool osmVisible;               ///< Tracks basemap visibility
    
    QPushButton* toggleBasemap;    ///< Button to switch basemap
    QVBoxLayout* layout;           ///< Widget layout

    void initCanvas(QWidget* containerFrame);  ///< Initializes map canvas
    void initLayers();                         ///< Loads basemap layers
    void connectSignals();                     ///< Connects UI signals
    void updateLabels();                       ///< Updates UI labels
    void toggleBaseLayer();                    ///< Toggles between basemaps
    QgsPointXY wgs84ToMercator(double lon, double lat);  ///< Coordinate conversion
};