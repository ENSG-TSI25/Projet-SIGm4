#include "./include/mainwindow.h"
#include <qgsapplication.h>
#include <QFile>
#include <QDebug>
#include <QTimer>  

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
    
    if (argc > 1) {
        QString filepath = QString::fromLocal8Bit(argv[1]);
        if (filepath.endsWith(".sigm4")) {
            qDebug() << "Loading project from command line:" << filepath;
            QTimer::singleShot(200, [&w, filepath]() {
                w.loadProject(filepath);
            });
        }
    }

    int r = a.exec();
    QgsApplication::exitQgis();
    return r;
}