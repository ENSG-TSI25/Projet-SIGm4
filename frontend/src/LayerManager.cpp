#include "../include/LayerManager.h"
#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>
#include <QWidget>

/**
 * @file LayerManager.cpp
 * @brief Implementation of LayerManager class
 * 
 * Contains file management, layer listing, and
 * layer operations for the GIS application.
 */

/**
 * @brief Constructor
 * @param mw Pointer to main window
 */
LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(" ")
{
}

/**
 * @brief Opens file dialog to select layer files
 * 
 * Shows Open File dialog for GeoPackage files
 * and updates the UI with selected file name.
 */
void LayerManager::listFiles(){
    mw->getUi()->selectedFileLabel->setText("");

    QWidget* parentWidget = qobject_cast<QWidget*>(parent());

    fileName = QFileDialog::getOpenFileName(
        parentWidget,
        tr("Open window"),
        "$PWD",
        tr("Files (*.gpkg)")
    );

    QStringList filenameChar = fileName.split(u'/');
    mw->getUi()->selectedFileLabel->setText(
        QString("Fichier sélectionné: %1").arg(filenameChar.last())
    );
    mw->getUi()->selectedFileLabel->setWordWrap(true);
}

/**
 * @brief Adds selected file to layer list widget
 * 
 * Creates a new item in the layers list with
 * the selected file name and checkable state.
 */
void LayerManager::addFileToWidget() { 
    if (!fileName.isEmpty()) {
        QStringList filenameChar = fileName.split(u'/');
        QString layerName = filenameChar.last();
        QListWidgetItem *item = new QListWidgetItem(layerName);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);

        mw->getUi()->layersList->addItem(item);

        fileName = "";
    }
}

/**
 * @brief Duplicates the selected layer
 * @param dialog Layer management dialog
 */
void LayerManager::duplicateLayer(Dialog* dialog) {
    QString name = dialog->nameLayer();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    int currentIndex = mw->getUi()->layersList->row(mw->getUi()->layersList->currentItem());
    mw->getUi()->layersList->insertItem(currentIndex, item);
}

/**
 * @brief Renames the selected layer
 * @param dialog Layer management dialog
 */
void LayerManager::renameLayer(Dialog* dialog) {
    duplicateLayer(dialog);
    int currentIndex = mw->getUi()->layersList->row(mw->getUi()->layersList->currentItem());
    mw->getUi()->layersList->takeItem(currentIndex);
}

/**
 * @brief Destructor
 */
LayerManager::~LayerManager() {}