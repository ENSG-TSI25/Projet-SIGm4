#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Carte.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void listFiles();
    void listDimension();
    void listSourceSys();
    void listTargetSys();
    void clickNewProject();

private:
    Ui::MainWindow *ui;

    Carte* carte;
};
#endif // MAINWINDOW_H
