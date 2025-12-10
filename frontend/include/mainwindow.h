#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include "Carte.h"
#include "Layer.h"
#include "../ui/ui_mainwindow.h"
#include "dialogLayerManagement.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Layer; 

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void listDimension();
    void openDialog();
    std::string selectCRSsource();
    std::string selectCRSdest();
    double getDate();
    std::tuple<std::string, std::string, double> transform();
    void setNewProject();
    void updateScaleLabel(int scaleValue);
    Ui::MainWindow* getUi();


private:
    Ui::MainWindow *ui;
    Carte* carte;
    Layer* layer;
    Dialog* dialog;
    void zoomIn_button();
    void zoomOut_button();
    void setCrsList(QComboBox *comboBox);

};
#endif // MAINWINDOW_H
