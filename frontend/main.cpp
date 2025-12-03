#include "src/mainwindow.h"
#include <qgsapplication.h>
#include "src/mainwindow.h"

int main(int argc, char *argv[])
{
    QgsApplication a(argc, argv, true);
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

    MainWindow w;
    w.show();

    int r = a.exec();
    QgsApplication::exitQgis();

    return r;
}
