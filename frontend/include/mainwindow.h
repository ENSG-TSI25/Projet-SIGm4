#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//Standard libraries
#include <string.h>
#include <iostream>

//Backend library
#include <core/DataManager.hpp>
#include <core/VectorLayer.hpp>
#include <core/RasterLayer.hpp>
#include <core/Project.hpp>

//Frontend files
#include "LayerManager.h"
#include "ProjectCaracteristicsDisplay.h"
#include "TransformCRS.h"
#include "../ui/ui_mainwindow.h"
#include "dialogLayerManagement.h"
#include "Carte.h"

//Qt library
#include <QWidget>
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QDoubleValidator>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QListWidget>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QDir>

//Qgis cpp API library
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsfield.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgsproject.h>

//Importing Gdal for updating the display of the project
#include <gdal_priv.h>    


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

    //Create a new project
    void setNewProject();
    //Open an already existing project from a .sigm4 file
    void loadProject(const QString& filepath);
    
    void updateScaleLabel(double scaleValue);
    void getCalendarDays(
    QCalendarWidget *calendar,
    QLabel *decimalDate,
    QLineEdit *epochEdit
    );

    float computeDate(int day, int month, int year);
    void updateSelectedDate(const QDate &date);
    LayerManager* getLayerManager(); 
    Project* getCurrentProject();
    Carte* getCarte();

    Ui::MainWindow* getUi();
    
    DataManager& getDataManager() { return dataManager; }

private:
    Ui::MainWindow *ui;
    Carte* carte;
    //Initialize to nullptr before the creation of a project
    Project* currentProject = nullptr;
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
    //Save the current project to a .sigm4 file
    void saveProject();
    //No parameters -slot for the "Open" button in UI
    void loadProject();
};
#endif // MAINWINDOW_H
