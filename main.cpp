#include "view/myview.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include<view/settingwidget.h>
#include<view/mainwindow.h>
#include"view/loginwidget.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow W;
//    W.show();
    MyView splash;
    splash.show();
//    SettingWidget s;
//    s.show();
//    LoginWidget W;
//    W.show();
    return a.exec();


}
