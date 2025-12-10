#include "./include/mainwindow.h"
#include <qgsapplication.h>
#include <QFile>
#include <QDebug>


int main(int argc, char *argv[])
{
    QgsApplication a(argc, argv, true);
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

    QFile styleFile(":/styles/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
        styleFile.close();
    }

    MainWindow w;
    w.show();

    int r = a.exec();
    QgsApplication::exitQgis();
    return r;
}
