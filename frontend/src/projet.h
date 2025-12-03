#include <string>

class Projet {
    private: 
        std::string projectCRS;
        double projectEpoch;
    public:
        void setProjectCrs();
        void setProjectEpoch();
        std::string getProjectCrs();
        double getProjectEpoch();
};