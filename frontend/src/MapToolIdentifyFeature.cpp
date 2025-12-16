#include "../include/MapToolIdentifyFeature.h"


#include <qgsmapmouseevent.h>  
#include <qgsfeature.h>
#include <qgsfields.h>
#include <QMessageBox>


MapToolIdentifyFeature::MapToolIdentifyFeature(
    QgsMapCanvas* canvas,
    QWidget* parent
)
    : QgsMapToolIdentify(canvas)
    , mParent(parent)
{
}

void MapToolIdentifyFeature::canvasReleaseEvent(QgsMapMouseEvent* e)
{
    QList<IdentifyResult> results =
        identify(
            e->x(),
            e->y(),
            TopDownStopAtFirst,
            VectorLayer
        );

    if (results.isEmpty())
        return;

    const IdentifyResult& res = results.first();

    QgsFeature feature = res.mFeature;
    QgsFields fields = feature.fields();

    QString text;
    for (int i = 0; i < fields.count(); ++i)
    {
        text += fields[i].name()
             + " : "
             + feature.attribute(i).toString()
             + "\n";
    }

    QMessageBox::information(
        mParent,
        "Attributs de l'entité",
        text
    );
}
