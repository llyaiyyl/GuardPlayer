#include "guardplayer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GuardPlayer w;
    w.show();

    return a.exec();
}
