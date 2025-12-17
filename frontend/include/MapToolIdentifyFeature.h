#pragma once

#include <qgsmaptoolidentify.h>
#include <QWidget>

class MapToolIdentifyFeature : public QgsMapToolIdentify
{
    Q_OBJECT

public:
    explicit MapToolIdentifyFeature(QgsMapCanvas* canvas, QWidget* parent = nullptr);

protected:
    void canvasReleaseEvent(QgsMapMouseEvent* e) override;

private:
    QWidget* mParent;
};
