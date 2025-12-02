#include "Carte.h"
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

Carte::Carte(QWidget* containerFrame)
    : osmVisible(true)
{
    initCanvas(containerFrame);
    initLayers();
    connectSignals();

    canvas->zoomToFullExtent();
    canvas->refresh();
}

Carte::~Carte() {}

QgsPointXY Carte::wgs84ToMercator(double lon, double lat)
{
    double x = lon * 20037508.34 / 180.0;
    double y = std::log(std::tan((90.0 + lat) * M_PI / 360.0)) / (M_PI / 180.0);
    y = y * 20037508.34 / 180.0;
    return QgsPointXY(x, y);
}

void Carte::initCanvas(QWidget* containerFrame)
{
    layout = new QVBoxLayout(containerFrame);
    layout->setContentsMargins(0,0,0,0);

    canvas = new QgsMapCanvas(containerFrame);
    canvas->setCanvasColor(Qt::white);
    canvas->enableAntiAliasing(true);

    QgsMapToolPan* panTool = new QgsMapToolPan(canvas);
    canvas->setMapTool(panTool);

    layout->addWidget(canvas);
}

void Carte::initLayers()
{
    // Basemap OSM
    osmLayer = new QgsRasterLayer(
        "type=xyz&url=https://tile.openstreetmap.org/{z}/{x}/{y}.png&zmax=19&zmin=0",
        "OSM",
        "wms"
    );

    // Basemap satellite
    satLayer = new QgsRasterLayer(
        "type=xyz&url=https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}&zmax=19&zmin=0",
        "Satellite",
        "wms"
    );

    osmLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));
    satLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:3857"));

    QgsProject::instance()->addMapLayer(osmLayer);
    QgsProject::instance()->addMapLayer(satLayer);


    // =========================
    //      LAYER FRANCE
    // =========================
    polyLayer = new QgsVectorLayer("Polygon?crs=EPSG:3857", "France", "memory");
    auto polyProv = polyLayer->dataProvider();

    QgsFeature france;
    france.setGeometry(QgsGeometry::fromPolygonXY({
        {
            wgs84ToMercator(-5.0, 41.0),
            wgs84ToMercator(9.5, 41.0),
            wgs84ToMercator(9.5, 51.0),
            wgs84ToMercator(-5.0, 51.0),
            wgs84ToMercator(-5.0, 41.0)
        }
    }));
    polyProv->addFeature(france);
    polyLayer->updateExtents();
    QgsProject::instance()->addMapLayer(polyLayer);

    // =========================
    //       POINTS
    // =========================
    pointLayer = new QgsVectorLayer("Point?crs=EPSG:3857", "Points", "memory");
    auto pointProv = pointLayer->dataProvider();

    QgsFeature f1; f1.setGeometry(QgsGeometry::fromPointXY(wgs84ToMercator(2.0, 48.0))); 
    pointProv->addFeature(f1);

    QgsFeature f2; f2.setGeometry(QgsGeometry::fromPointXY(wgs84ToMercator(4.0, 45.0))); 
    pointProv->addFeature(f2);

    pointLayer->updateExtents();
    QgsProject::instance()->addMapLayer(pointLayer);

    // =========================
    //       LIGNES
    // =========================
    lineLayer = new QgsVectorLayer("LineString?crs=EPSG:3857", "Lines", "memory");
    auto lineProv = lineLayer->dataProvider();

    QgsFeature l1;
    l1.setGeometry(QgsGeometry::fromPolylineXY({
        wgs84ToMercator(2,46),
        wgs84ToMercator(3,47),
        wgs84ToMercator(4,48)
    }));

    lineProv->addFeature(l1);
    lineLayer->updateExtents();
    QgsProject::instance()->addMapLayer(lineLayer);

    // =========================
    //     AJOUT AU CANVAS
    // =========================

    // ORDRE = du bas vers le haut
    canvas->setLayers({
        osmLayer,
        polyLayer,
        lineLayer,
        pointLayer
    });

    canvas->setExtent(polyLayer->extent());
    canvas->refresh();
}

void Carte::connectSignals()
{
    QObject::connect(canvas, &QgsMapCanvas::extentsChanged,
                     [this]() { updateLabels(); });

    QObject::connect(canvas, &QgsMapCanvas::mapCanvasRefreshed,
                     [this]() { updateLabels(); });
}

void Carte::updateLabels()
{
}
