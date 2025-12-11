#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include "Carte.h"
#include "LayerManager.h"
#include "TransformCRS.h"
#include "../ui/ui_mainwindow.h"
#include "dialogLayerManagement.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>
#include <QLabel>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QDate>

#include <string.h>

#include "../include/ProjectCaracteristicsDisplay.h"
#include <core/Project.hpp>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LayerManager; 

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void listDimension();
    void openDialog();
    void setNewProject();
    void openExistingProject();
    void saveCurrentProject();
    void updateScaleLabel(int scaleValue);
    void getCalendarDays(QCalendarWidget *calendar, QLabel *decimalDate);
    float computeDate(int day, int month, int year);
    // void displayEpochProject(const QDate &date);
    // void getSRCSelected();

    Ui::MainWindow* getUi();

private:
    Ui::MainWindow *ui;
    Carte* carte;
    Project* currentProject;
    LayerManager* layerManager;
    Dialog* dialog;
    TransformCRS* transform;
    ProjectCarateristicsDisplay* projectDisplay;
    void zoomIn_button();
    void zoomOut_button();
    void setCrsList(QComboBox *comboBox);

};
#endif // MAINWINDOW_H
