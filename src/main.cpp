#include <QApplication>
#include "ClockWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClockWidget w;
    w.resize(480, 480);
    w.show();
    
    // 在窗口显示后加载置顶设置
    w.loadAlwaysOnTopSetting();
    
    return a.exec();
}