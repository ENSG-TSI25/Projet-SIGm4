#include <core/TransformationEngine.hpp>
#include <vector>
#include <iostream>
#include <gdal/ogr_geometry.h>



VectorLayer* TransformationEngine::transformLayerAtEpoch(VectorLayer& inputLayer, const std::string& epsg_dst){
    std::vector<std::shared_ptr<Geometry4D>> vectorGeom = inputLayer.getGeometries();
    
    const std::string epsg_src = inputLayer.getCrs();
    const std::string epsg_src_clean = epsg_src.substr(epsg_src.find(":")+1);

    // Checks the coordinate format
    const std::string format = inputLayer.getCoordsType();
    std::cout<<"Type coordonnées : " << format << "\n";

    if (format == "cartesian"){
        // Projection from cartesian to geographic
        for (auto &g : vectorGeom){
            OGRGeometry* geom = g->getGeometry();
            OGRPoint* point = geom->toPoint();
        }
    }
    else if (format != "geographic"){
        std::cout<<"Format de coordonnées inconnu, arrêt de la transformation."<<"\n";
        return nullptr;
    }



    for (auto &g : vectorGeom){
        OGRGeometry* geom = g->getGeometry();
        OGRPoint* point = geom->toPoint();

        // Récupération des coordonnées et du temps
        const double t_init = g->getT();
        const double x_init = point->getX();
        const double y_init = point->getY();
        const double z_init = point->getZ();
        std::cout<<"Coordonnées avant projection : "<< x_init << " " << y_init << " " << z_init << " " << t_init << "\n";
        std::cout<<"EPSG source : "<< epsg_src_clean << ", EPSG destination : "<< epsg_dst << "\n";
        Result r = GeodeticTransformer::transformAtEpoch(x_init,y_init,z_init,t_init, epsg_src_clean, epsg_dst);
        double x_out = r.x;
        double y_out = r.y;
        double z_out = r.z;
        double t_out = r.t;
        g->setT(t_out);
        point->setX(x_out);
        point->setY(y_out);
        point->setZ(z_out);

        std::cout<<"Coordonnées après projection : "<< x_out << " " << y_out << " " << z_out << " " << t_out << "\n";
    }

    inputLayer.setCrs(epsg_dst);
    return &inputLayer;
}
