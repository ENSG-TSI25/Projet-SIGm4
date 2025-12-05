#include "dialog.h"
#include "ui_dialog.h"

#include <QLineEdit>
#include <QString>

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

QString Dialog::nameLayer() {
    QString name = dig -> lineEdit -> text();
    return name;
}

Ui::Dialog* Dialog::getUI() {
    return dig;
}
