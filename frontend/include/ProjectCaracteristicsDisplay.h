#ifndef PROJECTCARACTERISTICSDISPLAY_H
#define PROJECTCARACTERISTICSDISPLAY_H


#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include <string.h>

class MainWindow;


class ProjectCarateristicsDisplay: public QWidget
{
    Q_OBJECT

    private:
        std::string projectName;
        std::string projectCRS;
        double projectEpoch0;

        QLabel *nameLabel;
        QLabel *CRSLabel;
        QLabel *epoch0Label;

        

    public: 
        QVBoxLayout *layout;
        ProjectCarateristicsDisplay(MainWindow* mw);
        ~ProjectCarateristicsDisplay();
        void setProjectName(std::string newName);
        void setProjectEpoch0(double newEpoch0);
        void setProjectCRS(std::string newProject);
};

#endif // PROJECTCARACTERISTICSDISPLAY_H