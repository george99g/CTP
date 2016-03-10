#include "dialog.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qsrand(QDateTime::currentDateTime().toTime_t());
    Dialog w;
    w.show();
    return a.exec();
}
