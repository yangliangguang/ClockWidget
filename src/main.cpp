#include <QApplication>
#include "ClockWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClockWidget w;
    w.resize(480, 480);
    w.show();
    return a.exec();
}