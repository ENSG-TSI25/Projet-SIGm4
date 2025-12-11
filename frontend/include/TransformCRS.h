#ifndef TRANSFORMCRS_H
#define TRANSFORMCRS_H

#include <QObject>
#include <QString>
#include "../ui/ui_mainwindow.h"

class MainWindow;

/**
 * @file TransformCRS.h
 * @brief Coordinate transformation utilities
 * 
 * @class TransformCRS
 * @brief Handles coordinate system transformations
 * 
 * Manages conversion between different coordinate
 * reference systems using PROJ library.
 */
class TransformCRS : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param mw Pointer to main window
     */
    TransformCRS(MainWindow* mw);
    
    /**
     * @brief Destructor
     */
    ~TransformCRS();
    
    std::string selectCRSsource();  ///< Selects source CRS
    std::string selectCRSdest();    ///< Selects destination CRS
    double getDate();               ///< Gets transformation date
    std::tuple<std::string, std::string, double> transform(); ///< Performs transformation

private:
    MainWindow* mw;  ///< Main window reference
};

#endif // TRANSFORMCRS_H