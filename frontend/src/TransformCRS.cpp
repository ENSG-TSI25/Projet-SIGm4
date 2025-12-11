#include "../include/TransformCRS.h"
#include "../include/mainwindow.h"

/**
 * @file TransformCRS.cpp
 * @brief Implementation of TransformCRS class
 * 
 * Contains coordinate transformation logic
 * between different reference systems.
 */

/**
 * @brief Constructor
 * @param mw Pointer to main window
 */
TransformCRS::TransformCRS(MainWindow* mw) : QObject(mw), mw(mw) {
}

/**
 * @brief Destructor
 */
TransformCRS::~TransformCRS(){}

/**
 * @brief Gets selected source CRS from combo box
 * @return EPSG code as string
 */
std::string TransformCRS::selectCRSsource() {
    QString text = mw->getUi()->sourceCRSCombo->currentText();
    std::string crs = text.toStdString();
    int del = 0;
    for (int i = 0; i <= crs.length(); i++) {
        if (crs[i] == '(') {
            del = i + 1;
        }
    }
    crs.erase(0, del);
    crs.erase(crs.length() - 1);
    return crs;
}

/**
 * @brief Gets selected destination CRS from combo box
 * @return EPSG code as string
 */
std::string TransformCRS::selectCRSdest() {
    QString text = mw->getUi()->targetCRSCombo->currentText();
    std::string crs = text.toStdString();
    int del = 0;
    for (int i = 0; i <= crs.length(); i++) {
        if (crs[i] == '(') {
            del = i + 1;
        }
    }
    crs.erase(0, del);
    crs.erase(crs.length() - 1);
    return crs;
}

/**
 * @brief Gets transformation date from UI
 * @return Date as double
 */
double TransformCRS::getDate() {
    QString date = mw->getUi()->epochEdit->text();
    return date.toDouble();
}

/**
 * @brief Performs coordinate transformation
 * @return Tuple with source CRS, destination CRS, and date
 */
std::tuple<std::string, std::string, double> TransformCRS::transform() {
    std::tuple<std::string, std::string, double> final = {
        selectCRSsource(),
        selectCRSdest(), 
        getDate()
    };
    std::cout << std::get<0>(final);
    std::cout << std::get<1>(final);
    std::cout << std::get<2>(final);
    return final;
}