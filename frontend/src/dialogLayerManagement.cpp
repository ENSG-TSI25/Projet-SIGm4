#include "../include/dialogLayerManagement.h"
#include <QLineEdit>
#include <QString>

/**
 * @file dialogLayerManagement.cpp
 * @brief Implementation of Dialog class
 * 
 * Contains layer management dialog logic for
 * renaming and duplicating layers.
 */

/**
 * @brief Constructor - creates layer management dialog
 * @param parent Parent widget
 */
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    dig(new Ui::Dialog)
{
    dig->setupUi(this);
    connect(dig->lineEdit, &QLineEdit::textEdited, this, &Dialog::nameLayer);
}

/**
 * @brief Destructor
 */
Dialog::~Dialog()
{
    delete dig;
}

/**
 * @brief Gets the layer name from the text field
 * @return Layer name as QString
 */
QString Dialog::nameLayer() {
    QString name = dig->lineEdit->text();
    return name;
}

/**
 * @brief Gets the UI object
 * @return Pointer to dialog UI
 */
Ui::Dialog* Dialog::getUI() {
    return dig;
}