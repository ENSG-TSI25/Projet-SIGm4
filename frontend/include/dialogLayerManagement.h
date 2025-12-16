#ifndef DIALOG_H
#define DIALOG_H

//Frontend files
#include "../ui/ui_dialogLayerManagement.h" 

//Qt library
#include <QDialog>
#include <QLineEdit>
#include <QString>


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    QString nameLayer();
    Ui::Dialog* getUI();

private:
    Ui::Dialog *dig;
};

#endif // DIALOG_H
