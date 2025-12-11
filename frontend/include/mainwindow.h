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

#include <core/Project.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LayerManager;

/**
 * @file mainwindow.h
 * @brief Main application window
 * 
 * @class MainWindow
 * @brief Primary window of the GIS application
 * 
 * Contains all UI components: map, toolbar, layer panel,
 * coordinate transformation, and project management.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~MainWindow();
    
    void listDimension();                               ///< Lists available dimensions
    void openDialog();                                  ///< Opens layer dialog
    void setNewProject();                               ///< Creates new project
    void updateScaleLabel(int scaleValue);              ///< Updates scale display
    void getCalendarDays(QCalendarWidget *calendar, QLabel *decimalDate); ///< Gets dates from calendar
    float computeDate(int day, int month, int year);    ///< Computes decimal date
    void getDateSelected(const QDate &date);            ///< Handles date selection
    void updateSelectedDate(const QDate &date);         ///< Updates selected date
    void getSRCSelected();                              ///< Gets selected coordinate system

    /**
     * @brief Gets the UI object
     * @return Pointer to UI
     */
    Ui::MainWindow* getUi();

private:
    Ui::MainWindow *ui;         ///< UI object
    Carte* carte;               ///< Map widget
    Project* currentProject;    ///< Current GIS project
    LayerManager* layerManager; ///< Layer manager
    Dialog* dialog;             ///< Layer dialog
    TransformCRS* transform;    ///< Coordinate transformer

    void zoomIn_button();       ///< Zoom in action
    void zoomOut_button();      ///< Zoom out action
    void setCrsList(QComboBox *comboBox); ///< Populates CRS list
};

#endif // MAINWINDOW_H