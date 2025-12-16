#ifndef TRANSFORMCRS_H
#define TRANSFORMCRS_H

//Frontend files
#include "../ui/ui_mainwindow.h"

//Qt library
#include <QObject>
#include <QString>


class MainWindow;

class TransformCRS : public QObject
{
    Q_OBJECT

public:
    TransformCRS(MainWindow* mw);
    ~TransformCRS();
    std::string selectCRSsource();
    std::string selectCRSdest();
    double getDate();
    std::tuple<std::string, std::string, double> transform();
  

private:
    MainWindow* mw;

};
#endif // TRANSFORMCRS_H
