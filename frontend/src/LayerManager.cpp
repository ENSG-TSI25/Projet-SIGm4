#include "../include/LayerManager.h"
#include <QMainWindow>
#include <QString>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <qgsmapcanvas.h>
#include <QWidget>


LayerManager::LayerManager(MainWindow* mw) : QObject(mw), mw(mw), fileName(" ")
{
}



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


//Duplicate the layer when it's clicked
void LayerManager::duplicateLayer(Dialog* dialog) {
    QString name = dialog-> nameLayer();
    QListWidgetItem *item = new QListWidgetItem(name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    int currentIndex = mw->getUi() -> layersList -> row(mw->getUi() -> layersList -> currentItem());
    mw->getUi() -> layersList -> insertItem(currentIndex, item);
}

//Rename layer selected
void LayerManager::renameLayer(Dialog* dialog) {
    duplicateLayer(dialog);
    int currentIndex = mw->getUi() -> layersList -> row(mw->getUi() -> layersList -> currentItem());
    mw->getUi() -> layersList -> takeItem(currentIndex);
}


LayerManager::~LayerManager() {}
