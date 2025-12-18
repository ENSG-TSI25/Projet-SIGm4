#include "../include/DeformationModelWidget.h"
#include <core/GeodeticTransformer.hpp>
#include <QVBoxLayout>
#include <QDebug>

DeformationModelWidget::DeformationModelWidget(QWidget *parent)
    : QWidget(parent), currentEPSG(-1)
{
    setupUI();
}

void DeformationModelWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Label pour affichage d'un modèle unique
    modelLabel = new QLabel(this);
    modelLabel->setStyleSheet("QLabel { color: #0066cc; font-weight: bold; }");
    modelLabel->hide();
    layout->addWidget(modelLabel);
    
    // ComboBox pour choix multiple
    modelCombo = new QComboBox(this);
    modelCombo->hide();
    connect(modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeformationModelWidget::onModelComboChanged);
    layout->addWidget(modelCombo);
    
    setLayout(layout);
}

void DeformationModelWidget::updateForEPSG(int epsgCode)
{
    currentEPSG = epsgCode;
    QStringList models = getDeformationModels(epsgCode);
    
    // Cache tout par défaut
    modelLabel->hide();
    modelCombo->hide();
    selectedModel.clear();
    
    if (models.isEmpty())
    {
        // Aucun modèle : on n'affiche rien
        return;
    }
    else if (models.size() == 1)
    {
        // Un seul modèle : affichage en label
        modelLabel->setText(QString("Modèle de déformation: %1").arg(models.first()));
        modelLabel->show();
        selectedModel = models.first();
    }
    else
    {
        // Plusieurs modèles : liste déroulante
        modelCombo->clear();
        modelCombo->addItems(models);
        modelCombo->show();
        selectedModel = models.first();
    }
}

QString DeformationModelWidget::getSelectedModel() const
{
    return selectedModel;
}

bool DeformationModelWidget::hasModel() const
{
    return !selectedModel.isEmpty();
}

void DeformationModelWidget::onModelComboChanged(int index)
{
    if (index >= 0)
    {
        selectedModel = modelCombo->currentText();
        emit modelChanged(selectedModel);
    }
}

QStringList DeformationModelWidget::getDeformationModels(int epsgCode)
{
    QStringList models;
    
    // Recherche dans le registre 2D
    const auto& reg2D = GeodeticTransformer::getRegistry2D();
    auto it2D = reg2D.find(epsgCode);
    if (it2D != reg2D.end())
    {
        for (const auto& model : it2D->second.deformation_models)
        {
            if (!model.empty() && model != "deformation=0")
            {
                models.append(QString::fromStdString(model));
            }
        }
    }
    
    // Recherche dans le registre 3D
    const auto& reg3D = GeodeticTransformer::getRegistry3D();
    auto it3D = reg3D.find(epsgCode);
    if (it3D != reg3D.end())
    {
        for (const auto& model : it3D->second.deformation_models)
        {
            if (!model.empty() && model != "deformation=0" && !models.contains(QString::fromStdString(model)))
            {
                models.append(QString::fromStdString(model));
            }
        }
    }
    
    // Recherche dans le registre projeté
    const auto& regProj = GeodeticTransformer::getRegistryProjected();
    auto itProj = regProj.find(epsgCode);
    if (itProj != regProj.end())
    {
        for (const auto& model : itProj->second.deformation_models)
        {
            if (!model.empty() && model != "deformation=0" && !models.contains(QString::fromStdString(model)))
            {
                models.append(QString::fromStdString(model));
            }
        }
    }
    
    return models;
}