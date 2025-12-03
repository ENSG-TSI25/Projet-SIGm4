#include "src/mainwindow.h"

#include <QApplication>
#include <QDialog>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //On crée la boîte de dialogue
    QDialog chosingCRSDialog;
    chosingCRSDialog.setWindowTitle("Choix du CRS");
    QVBoxLayout *layout = new QVBoxLayout(&chosingCRSDialog);
    QLabel *dialogText = new QLabel("Choisissez un CRS et une époque pour votre projet", &chosingCRSDialog);
    QPushButton *acceptationButton = new QPushButton("OK", &chosingCRSDialog);

    //on ajoute les widgets
    layout->addWidget(dialogText);
    layout->addWidget(acceptationButton);

    QObject::connect(acceptationButton, &QPushButton::clicked, &chosingCRSDialog, &QDialog::accept);


    //Une fois que l'utilisateur a accepté, on peut lancer la MainWindow
    if (chosingCRSDialog.exec() == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec();
    }
    return 0;

}
