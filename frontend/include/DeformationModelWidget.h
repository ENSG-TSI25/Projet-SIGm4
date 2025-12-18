#ifndef DEFORMATIONMODELWIDGET_H
#define DEFORMATIONMODELWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QString>
#include <QStringList>

// Forward declaration
class GeodeticTransformer;

class DeformationModelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeformationModelWidget(QWidget *parent = nullptr);
    
    // Update the widget based on the provided EPSG code
    void updateForEPSG(int epsgCode);
    
    // Return the currently selected deformation model
    QString getSelectedModel() const;
    
    // Check if a deformation model is selected
    bool hasModel() const;

signals:
    void modelChanged(const QString &model);

private slots:
    void onModelComboChanged(int index);

private:
    void setupUI();
    
    // Retrieve the deformation models for a given EPSG code
    QStringList getDeformationModels(int epsgCode);

    QLabel *modelLabel;
    QComboBox *modelCombo;
    QString selectedModel;
    int currentEPSG = -1;
};

#endif // DEFORMATIONMODELWIDGET_H