#include "./include/mainwindow.h"
#include <qgsapplication.h>
#include <QFile>
#include <QDebug>

/**
 * @file main.cpp
 * @brief Main entry point for the GIS application
 * 
 * Initializes Qt and QGIS frameworks, loads application styles,
 * creates and shows the main window, and starts the event loop.
 * This is where the program execution begins.
 */

/**
 * @brief Main function - application entry point
 * @param argc Argument count
 * @param argv Argument values array
 * @return Application exit code (0 for success, non-zero for error)
 * 
 * This function performs the following steps:
 * 1. Initializes QGIS application with prefix path
 * 2. Loads and applies CSS stylesheet for UI styling
 * 3. Creates the main window and displays it
 * 4. Starts the Qt event loop
 * 5. Cleans up QGIS resources before exit
 */
int main(int argc, char *argv[])
{
    // Initialize QGIS application
    QgsApplication a(argc, argv, true);
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

    // Load and apply CSS stylesheet
    QFile styleFile(":/styles/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    }

    // Create and show main window
    MainWindow w;
    w.show();

    // Start event loop and clean up
    int r = a.exec();
    QgsApplication::exitQgis();
    return r;
}