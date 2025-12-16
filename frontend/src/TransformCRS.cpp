#include "../include/TransformCRS.h"
#include "../include/mainwindow.h"

TransformCRS::TransformCRS(MainWindow* mw) : QObject(mw), mw(mw) {
}

TransformCRS::~TransformCRS(){}

std::string TransformCRS::selectCRSsource() {
    QString text = mw->getUi()->sourceCRSCombo->currentText();
    std::string crs = text.toStdString();
    int del = 0;
    for (int i=0; i<=crs.length();i++) {
        if (crs[i] == '(') {
            del = i+1;
        }
    }
    crs.erase(0,del);
    crs.erase(crs.length()-1);
    return crs;
}

std::string TransformCRS::selectCRSdest() {
    QString text = mw->getUi() -> targetCRSCombo->currentText();
    std::string crs = text.toStdString();
    int del = 0;
    for (int i=0; i<=crs.length();i++) {
        if (crs[i] == '(') {
            del = i+1;
        }
    }
    crs.erase(0,del);
    crs.erase(crs.length()-1);
    return crs;
}

double TransformCRS::getDate() {
    QString date = mw->getUi() -> epochEdit -> text();
    return date.toDouble();
}

std::tuple<std::string, std::string, double> TransformCRS::transform() {
    std::tuple<std::string, std::string, double> final = {selectCRSsource(),selectCRSdest(), getDate()};
    std::cout<<std::get<0>(final);
    std::cout<<std::get<1>(final);
    std::cout<<std::get<2>(final);
    return final;
}