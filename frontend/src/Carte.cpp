#include "../include/Carte.h"


Carte::Carte(QWidget* containerFrame, MainWindow* mw)
    : osmVisible(true) , QObject(mw), mw(mw)
{
    initCanvas(containerFrame);
    initLayers();
    connectSignals();

    canvas->zoomToFullExtent();
    canvas->refresh();
}
Carte::~Carte() {}


void Carte::initCanvas(QWidget* containerFrame)
{


    layout = new QVBoxLayout(containerFrame);
    layout->setContentsMargins(0,0,0,0);

    toggleBasemap = new QPushButton("Basemap OSM / Satellite");
    layout->addWidget(toggleBasemap);
    

    canvas = new QgsMapCanvas(containerFrame);
    canvas->setCanvasColor(Qt::white);
    canvas->enableAntiAliasing(true);

    canvas->setDestinationCrs(QgsCoordinateReferenceSystem(QString::fromStdString(getCarteEpsg())));

    QgsMapToolPan* panTool = new QgsMapToolPan(canvas);
    canvas->setMapTool(panTool);

    layout->addWidget(canvas);

}

void Carte::initLayers()
{
    canvas->setDestinationCrs(QgsCoordinateReferenceSystem(QString::fromStdString(getCarteEpsg())));

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

    QgsCoordinateReferenceSystem webMercator("EPSG:3857");

    osmLayer->setCrs(webMercator);
    satLayer->setCrs(webMercator);

    QgsProject::instance()->addMapLayer(osmLayer);
    QgsProject::instance()->addMapLayer(satLayer);


    // ============================
    //      ADD BASE LAYER
    // ============================
    osmVisible = true;
    canvas->setLayers({osmLayer});
    canvas->setExtent(osmLayer->extent());
    canvas->refresh();
}

void Carte::connectSignals()
{

    QObject::connect(toggleBasemap, &QPushButton::clicked,
                     this, &Carte::toggleBaseLayer);
}


void Carte::toggleBaseLayer()
{
    osmVisible = !osmVisible;

    QgsMapCanvas* c = canvas;
    QList<QgsMapLayer*> layers = c->layers();

    layers.removeAll(osmLayer);
    layers.removeAll(satLayer);

    if (osmVisible)
    {
        layers.append(osmLayer);
    }
    else
    {
        layers.append(satLayer);
    }

    c->setLayers(layers);
    c->refresh();
}


void Carte::clearUserLayers()
    {
        // Get all layers from the project
        QMap<QString, QgsMapLayer*> allLayers = QgsProject::instance()->mapLayers();
        QStringList layersToRemove;
        
        // Collect all layers except OSM and Satellite
        for (auto it = allLayers.begin(); it != allLayers.end(); ++it)
        {
            QgsMapLayer* layer = it.value();
            if (layer != osmLayer && layer != satLayer)
            {
                layersToRemove << it.key();
            }
        }
        
        // Remove all non-base layers from the project
        QgsProject::instance()->removeMapLayers(layersToRemove);
        
        // Update canvas to show only the current base layer
        if (osmVisible)
        {
            canvas->setLayers({osmLayer});
        }
        else
        {
            canvas->setLayers({satLayer});
        }
        
        canvas->refresh();
    }