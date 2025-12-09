#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

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
