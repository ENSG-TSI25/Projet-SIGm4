#include "src/mainwindow.h"

#include <QApplication>
#include <QDialog>
#include <QLineEdit>
#include <QDoubleValidator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

        MainWindow w;
        w.show();
        return a.exec();
    return 0;

    int r = a.exec();
    QgsApplication::exitQgis();

    return r;
}
