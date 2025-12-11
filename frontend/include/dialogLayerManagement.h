#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "../ui/ui_dialogLayerManagement.h" 

namespace Ui {
class Dialog;
}

/**
 * @file dialogLayerManagement.h
 * @brief Layer management dialog
 * 
 * @class Dialog
 * @brief Dialog window for managing map layers
 * 
 * Allows users to add, rename, and configure layers
 * through a graphical interface.
 */
class Dialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit Dialog(QWidget *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~Dialog();
    
    /**
     * @brief Gets the layer name from dialog
     * @return Layer name as QString
     */
    QString nameLayer();
    
    /**
     * @brief Gets the UI object
     * @return Pointer to UI
     */
    Ui::Dialog* getUI();

private:
    Ui::Dialog *dig;  ///< UI object
};

#endif // DIALOG_H