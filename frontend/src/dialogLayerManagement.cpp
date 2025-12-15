#include "../include/dialogLayerManagement.h"

#include <QLineEdit>
#include <QString>

//Create a dialog class to manage the layers
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    dig(new Ui::Dialog)
{
    dig->setupUi(this);

    connect(dig->lineEdit, &QLineEdit::textEdited, this, &Dialog::nameLayer);
}

Dialog::~Dialog()
{
    delete dig;
}

//Taking the layer's name
QString Dialog::nameLayer() {
    QString name = dig -> lineEdit -> text();
    return name;
}

//Return dialog's UI to place in the main window
Ui::Dialog* Dialog::getUI() {
    return dig;
} 
