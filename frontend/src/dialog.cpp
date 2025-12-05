#include "dialog.h"
#include "ui_dialog.h"

#include <QLineEdit>
#include <QString>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    dig(new Ui::Dialog)
{
    dig->setupUi(this);

    connect(dig->lineEdit, &QLineEdit::textEdited, this, &Dialog::renameLayer);
}

Dialog::~Dialog()
{
    delete dig;
}

QString Dialog::renameLayer() {
    QString name = dig -> lineEdit -> text();
    return name;
}
