#include <string>

class Projet {
    private: 
        std::string projectCRS;
        double projectEpoch;
    public:
        void setProjectCrs(std::string CRS);
        void setProjectEpoch(double epoch);
        std::string getProjectCrs();
        double getProjectEpoch();
};