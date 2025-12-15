#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QComboBox>
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

#include <core/DataManager.hpp>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LayerManager; 
class Carte;

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
    void getCalendarDays(
    QCalendarWidget *calendar,
    QLabel *decimalDate,
    QLineEdit *epochEdit
);

    float computeDate(int day, int month, int year);
    void getDateSelected(const QDate &date);
    void updateSelectedDate(const QDate &date);
    void getSRCSelected();
    LayerManager* getLayerManager(); 
    Project* getCurrentProject();
    Carte* getCarte();
    void loadProject(const QString& filepath);

    Ui::MainWindow* getUi();
    
    DataManager& getDataManager() { return dataManager; }




private:
    Ui::MainWindow *ui;
    Carte* carte;
    Project* currentProject;
    LayerManager* layerManager;
    Dialog* dialog;
    TransformCRS* transform;
    ProjectCarateristicsDisplay* projectDisplay;
    void setProjectActionsEnabled(bool enabled);
    void zoomIn_button();
    void zoomOut_button();
    void setCrsList(QComboBox *comboBox);
    DataManager dataManager;


private slots:
    void saveProject();
    void loadProject();
};
#endif // MAINWINDOW_H