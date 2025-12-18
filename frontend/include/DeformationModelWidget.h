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
    
    // Met à jour l'affichage en fonction du code EPSG sélectionné
    void updateForEPSG(int epsgCode);
    
    // Retourne le modèle sélectionné (vide si aucun)
    QString getSelectedModel() const;
    
    // Vérifie si un modèle est disponible/sélectionné
    bool hasModel() const;

signals:
    void modelChanged(const QString &model);

private slots:
    void onModelComboChanged(int index);

private:
    void setupUI();
    
    // Récupère les modèles de déformation pour un EPSG donné
    QStringList getDeformationModels(int epsgCode);

    QLabel *modelLabel;
    QComboBox *modelCombo;
    QString selectedModel;
    int currentEPSG = -1;
};

#endif // DEFORMATIONMODELWIDGET_H