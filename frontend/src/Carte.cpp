#include "../include/Carte.h"
#include <cmath>
#include <QVBoxLayout>
#include <QDebug>

#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsproject.h>
#include <qgsmaptoolpan.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>

/**
 * @file Carte.cpp
 * @brief Implementation of Carte class
 * 
 * Contains map display logic, layer management,
 * and coordinate transformations for the GIS application.
 */

/**
 * @brief Constructor - initializes map widget
 * @param containerFrame Parent widget for the map canvas
 */
Carte::Carte(QWidget* containerFrame)
    : osmVisible(true)
{
    initCanvas(containerFrame);
    initLayers();
    connectSignals();

    canvas->zoomToFullExtent();
    canvas->refresh();
}

/**
 * @brief Destructor
 */
Carte::~Carte() {}

/**
 * @brief Converts WGS84 coordinates to Web Mercator
 * @param lon Longitude in degrees
 * @param lat Latitude in degrees
 * @return QgsPointXY in EPSG:3857 (Web Mercator)
 */
QgsPointXY Carte::wgs84ToMercator(double lon, double lat)
{
    double x = lon * 20037508.34 / 180.0;
    double y = std::log(std::tan((90.0 + lat) * M_PI / 360.0)) / (M_PI / 180.0);
    y = y * 20037508.34 / 180.0;
    return QgsPointXY(x, y);
}

/**
 * @brief Initializes the map canvas and UI
 * @param containerFrame Parent widget
 */
void Carte::initCanvas(QWidget* containerFrame)
{
    layout = new QVBoxLayout(containerFrame);
    layout->setContentsMargins(0,0,0,0);

    toggleBasemap = new QPushButton("Basemap OSM / Satellite");
    layout->addWidget(toggleBasemap);
    
    canvas = new QgsMapCanvas(containerFrame);
    canvas->setCanvasColor(Qt::white);
    canvas->enableAntiAliasing(true);
    canvas->setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    QgsMapToolPan* panTool = new QgsMapToolPan(canvas);
    canvas->setMapTool(panTool);

    layout->addWidget(canvas);
}

/**
 * @brief Initializes basemap layers
 * 
 * Loads OpenStreetMap and satellite imagery layers
 * and sets up the map display.
 */
void Carte::initLayers()
{
    canvas->setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    osmLayer = new QgsRasterLayer(
        "type=xyz&url=https://tile.openstreetmap.org/{z}/{x}/{y}.png&zmax=19&zmin=0",
        "OSM",
        "wms"
    );

    satLayer = new QgsRasterLayer(
        "type=xyz&url=https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}&zmax=19&zmin=0",
        "Satellite",
        "wms"
    );

    osmLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    satLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    QgsProject::instance()->addMapLayer(osmLayer);
    QgsProject::instance()->addMapLayer(satLayer);

    osmVisible = true;
    canvas->setLayers({osmLayer});
    canvas->setExtent(osmLayer->extent());
    canvas->refresh();
}

/**
 * @brief Connects UI signals to slots
 */
void Carte::connectSignals()
{
    QObject::connect(toggleBasemap, &QPushButton::clicked,
                     this, &Carte::toggleBaseLayer);
}

/**
 * @brief Toggles between OSM and satellite basemaps
 */
void Carte::toggleBaseLayer()
{
    osmVisible = !osmVisible;

    if(osmVisible)
    {
        canvas->setLayers({ osmLayer });
    }
    else
    {
        canvas->setLayers({ satLayer });
    }

    canvas->refresh();
}