#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include "Carte.h"
#include "dialogLayerManagement.h"
#include "ui_dialogLayerManagement.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void listFiles();
    void listDimension();
    void addFileToWidget();
    void openDialog();
    void duplicateLayer();
    void renameLayer();
    std::string selectCRSsource();
    std::string selectCRSdest();
    double getDate();
    std::tuple<std::string, std::string, double> transform();
    //void updateScaleLabel(double scaleValue);
    void setNewProject();
    void updateScaleLabel(int scaleValue);

private:
    Ui::MainWindow *ui;
    Carte* carte;
    Dialog* dialog;
    QString fileName;
    void zoomIn_button();
    void zoomOut_button();
    void setCrsList(QComboBox *comboBox);

};
#endif // MAINWINDOW_H
