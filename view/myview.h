#ifndef MYVIEW_H
#define MYVIEW_H
#include<QGraphicsView>//视图
#include <QObject>
#include<QGraphicsScene>//场景
#include "myitem.h"//图元
#include <QTimer>//定时器
#include<QSettings>
#include<QtSerialPort/QSerialPortInfo>
#include<QCameraInfo>
#include<view/settingwidget.h>
#include<view/mainwindow.h>
class MyView:public QGraphicsView
{
    Q_OBJECT
public:
    MyView();
private:
    QGraphicsScene * scene;
    MyItem * item1, * item2, * item3;
    QTimer * mytimer;
    QTimer * delayTime;
    SettingWidget * settingWin= nullptr;
    MainWindow *mainWin;
public slots:
    void stopTimer();
    void DelayFinish();
    void onConfigSaved();
private:
    bool isConfigValid();
};

#endif // MYVIEW_H
