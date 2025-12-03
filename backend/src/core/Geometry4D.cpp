#include <core/Geometry4D.hpp>

Geometry4D::Geometry4D() : geometry(nullptr), crs("EPSG:4326") {}

Geometry4D::Geometry4D(OGRGeometry* geom) : geometry(geom), crs("EPSG:4326") {
    if (geometry && !geometry->IsMeasured()) {
        geometry->set3D(TRUE);
        geometry->setMeasured(TRUE);
    }
}

Geometry4D::Geometry4D(const Geometry4D& other) : crs(other.crs) {
    if (other.geometry) {
        geometry.reset(other.geometry->clone());
    }
}

Geometry4D& Geometry4D::operator=(const Geometry4D& other) {
    if (this != &other) {
        crs = other.crs;
        geometry.reset(other.geometry ? other.geometry->clone() : nullptr);
    }
    return *this;
}

double Geometry4D::getT() const {
    if (!geometry || !geometry->IsMeasured()) return 0.0;
    auto type = wkbFlatten(geometry->getGeometryType());
    if (type == wkbPoint) return geometry->toPoint()->getM();
    if (type == wkbLineString) return geometry->toLineString()->getM(0);
    return 0.0;
}

void Geometry4D::setT(double ts) {
    if (!geometry) return;
    auto type = wkbFlatten(geometry->getGeometryType());
    if (type == wkbPoint) geometry->toPoint()->setM(ts);
    else if (type == wkbLineString) {
        auto* line = geometry->toLineString();
        for (int i = 0; i < line->getNumPoints(); ++i) line->setM(i, ts);
    }
}

int Geometry4D::getSRID() const {
    if (!geometry) return 0;
    auto* srs = geometry->getSpatialReference();
    if (!srs) return 0;
    auto* code = srs->GetAuthorityCode(nullptr);
    return code ? std::atoi(code) : 0;
}

void Geometry4D::setSRID(int srid) {
    if (!geometry) return;
    
    OGRSpatialReference* srs = new OGRSpatialReference();
    srs->importFromEPSG(srid);
    geometry->assignSpatialReference(srs);
    srs->Release();
    
    crs = "EPSG:" + std::to_string(srid);
}