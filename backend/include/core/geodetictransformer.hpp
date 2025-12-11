#ifndef GEODETICTRANSFORMER_H
#define GEODETICTRANSFORMER_H

#pragma once
#include <string>
#include <proj.h>

/**
 * @class GeodeticTransformer
 * @brief Classe responsable des transformations géodésiques spatio-temporelles
 * 
 * Cette classe fournit des méthodes pour effectuer des transformations
 * géodésiques prenant en compte le temps (époques), incluant les déformations
 * crustales et les transformations entre systèmes de référence.
 * @note Utilise la bibliothèque PROJ pour les transformations de coordonnées
 */
class GeodeticTransformer {
public:
    /**
     * @brief Constructeur par défaut
     * Initialise le contexte PROJ
     */
    GeodeticTransformer();
    
    /**
     * @brief Destructeur
     * Nettoie le contexte PROJ
     */
    ~GeodeticTransformer();

    /**
     * @struct Result
     * @brief Structure contenant les résultats d'une transformation 4D
     */
    struct Result {
        double x; /**< Coordonnée X */
        double y; /**< Coordonnée Y */ 
        double z; /**< Coordonnée Z */
        double t; /**< Temps/époque */
    };

    /**
     * @brief Transformation dépendante du temps entre systèmes de référence
     * 
     * @param x Coordonnée X source
     * @param y Coordonnée Y source
     * @param z Coordonnée Z source
     * @param t_epoch Époque de la donnée source (en années décimales)
     * @param epsg_src Code EPSG du système source (ex: "EPSG:4326")
     * @param epsg_dst Code EPSG du système cible
     * @return Result Coordonnées transformées avec époque
     * 
     * @example
     * @code
     * GeodeticTransformer gt;
     * auto result = gt.transformAtEpoch(10.0, 45.0, 100.0, 2020.5, 
     *                                   "EPSG:4326", "EPSG:32738");
     * @endcode
     */
    Result transformAtEpoch(
        double x, double y, double z, double t_epoch,
        const std::string& epsg_src,
        const std::string& epsg_dst);

    /**
     * @brief Applique un modèle de déformation depuis un fichier JSON
     * 
     * Utilise +proj=defmodel pour appliquer des déformations crustales
     * spécifiques à une région (ex: Mayotte)
     * 
     * @param lon Longitude (degrés)
     * @param lat Latitude (degrés)
     * @param h Hauteur ellipsoïdale (mètres)
     * @param t_epoch Époque de la donnée
     * @param json_model_path Chemin vers le fichier JSON du modèle
     * @param inverse Si true, applique la transformation inverse
     * @return Result Coordonnées après déformation
     * 
     * @warning Le fichier JSON doit suivre le format PROJ defmodel
     */
    Result applyDefModel(
        double lon, double lat, double h, double t_epoch,
        const std::string& json_model_path,
        bool inverse = false);

    /**
     * @brief Applique une déformation via grille de déplacement
     * 
     * Utilise +proj=deformation avec une grille de déformation au format NTv2
     * ou similaire pour appliquer des déformations locales
     * 
     * @param x Coordonnée X
     * @param y Coordonnée Y
     * @param z Coordonnée Z
     * @param t_epoch Époque de la donnée
     * @param grid_path Chemin vers le fichier de grille
     * @param ref_epoch Époque de référence de la grille
     * @return Result Coordonnées déformées
     * 
     * @note Les grilles de déformation doivent être compatibles PROJ
     */
    Result applyGridDeformation(
        double x, double y, double z, double t_epoch,
        const std::string& grid_path,
        double ref_epoch);

private:
    PJ_CONTEXT* ctx_; /**< Contexte PROJ pour les opérations de transformation */
};

#endif // GEODETICTRANSFORMER_H