#ifndef PROJECTCARACTERISTICSDISPLAY_H
#define PROJECTCARACTERISTICSDISPLAY_H


#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include <core/Project.hpp>

class MainWindow;


class ProjectCarateristicsDisplay: public QWidget
{
    Q_OBJECT

    private:
        MainWindow *mw;
        QLabel *nameLabel;
        QLabel *CRSLabel;
        QLabel *epoch0Label;      

    public: 
        QVBoxLayout *layout;
        ProjectCarateristicsDisplay(MainWindow* mw);
        ~ProjectCarateristicsDisplay();
        void updateDisplayName();
        void updateDisplayEpoch0();
        void updateDisplayCRS();
};

#endif // PROJECTCARACTERISTICSDISPLAY_H