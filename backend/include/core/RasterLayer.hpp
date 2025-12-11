#pragma once
#include <core/Layer.hpp>
#include <core/Geometry4D.hpp>
#include <memory>

/**
 * @class RasterLayer
 * @brief Classe représentant une couche raster dans le SIG
 * 
 * Hérite de Layer et ajoute des fonctionnalités spécifiques aux données raster
 * telles que la géoréférencement et les dimensions de l'image
 * 
 * @see Layer
 * @see Geometry4D
 */
class RasterLayer : public Layer {
private:
    std::shared_ptr<Geometry4D> emprise; /**< Emprise 4D de la couche raster */
    std::string filePath; /**< Chemin vers le fichier raster */
    double geoTransform[6]; /**< Matrice de transformation géographique (GDAL-like) */
    int width; /**< Largeur de l'image en pixels */
    int height; /**< Hauteur de l'image en pixels */

public:
    /**
     * @brief Constructeur de RasterLayer
     * 
     * @param nom_ Nom de la couche
     * @param crs_ Système de coordonnées (par défaut: EPSG:4326)
     * @param epoque_ Époque de la donnée (par défaut: 0.0)
     */
    RasterLayer(const std::string& nom_, const std::string& crs_ = "EPSG:4326", 
                double epoque_ = 0.0)
        : Layer(nom_, crs_, epoque_), width(0), height(0) {
        std::fill(geoTransform, geoTransform + 6, 0.0);
    }
    
    /**
     * @brief Définit l'emprise 4D de la couche
     * 
     * @param bbox Pointeur vers l'objet Geometry4D décrivant l'emprise
     */
    void setEmprise(std::shared_ptr<Geometry4D> bbox) { emprise = bbox; }
    
    /**
     * @brief Définit le chemin vers le fichier raster
     * 
     * @param path Chemin du fichier
     */
    void setFilePath(const std::string& path) { filePath = path; }
    
    /**
     * @brief Définit la matrice de géotransformation
     * 
     * Format GDAL : [X_origine, taille_pixel_X, rotation_X, 
     *                Y_origine, rotation_Y, taille_pixel_Y]
     * 
     * @param gt Tableau de 6 doubles contenant la géotransformation
     */
    void setGeoTransform(const double* gt) { 
        std::copy(gt, gt + 6, geoTransform); 
    }
    
    /**
     * @brief Définit les dimensions de l'image raster
     * 
     * @param w Largeur en pixels
     * @param h Hauteur en pixels
     */
    void setDimensions(int w, int h) { width = w; height = h; }
    
    /**
     * @brief Récupère l'emprise 4D de la couche
     * 
     * @return Pointeur vers l'emprise Geometry4D
     */
    std::shared_ptr<Geometry4D> getEmprise() const { return emprise; }
    
    /**
     * @brief Récupère le chemin du fichier raster
     * 
     * @return Chemin du fichier
     */
    std::string getFilePath() const { return filePath; }
    
    /**
     * @brief Récupère la matrice de géotransformation
     * 
     * @return Pointeur constant vers le tableau de géotransformation
     */
    const double* getGeoTransform() const { return geoTransform; }
    
    /**
     * @brief Récupère la largeur de l'image
     * 
     * @return Largeur en pixels
     */
    int getWidth() const { return width; }
    
    /**
     * @brief Récupère la hauteur de l'image
     * 
     * @return Hauteur en pixels
     */
    int getHeight() const { return height; }
    
    /**
     * @brief Calcule la résolution du raster
     * 
     * @return std::pair<double, double> Résolution (X, Y) en unités du CRS
     */
    std::pair<double, double> getResolution() const {
        return std::make_pair(geoTransform[1], std::abs(geoTransform[5]));
    }
};