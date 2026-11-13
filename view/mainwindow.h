#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSettings>
#include <QCamera>
#include <QMessageBox>
#include "settingwidget.h"
#include<QPixmap>
#include<QSerialPort>
#include<QSerialPortInfo>
#include<QString>
#include <QDebug>
#include<QList>
#include<QtCharts/QChart>
#include<QtCharts/QChartView>
#include<QtCharts/QSplineSeries>
#include<QtCharts/QDateTimeAxis>
#include<QtCharts/QValueAxis>
#include<QDateTime>
#include<data/singletondb.h>
#include<QtCharts/QBarSeries>
#include<QtCharts/QBarSet>  //创建柱状图
#include<QtCharts/QCategoryAxis>
#include<QStackedLayout>
#include<thread/camerathread.h>
#include <QTableView>
#include <QHeaderView>
#include"Control/devicecontrol.h"
#include<QTableWidget>
#include<QPieSeries>//创建饼图数据系列
#include"data/alarminfo.h"
#include"view/loginwidget.h"
#include"view/videoplaybackwidget.h"
using namespace QtCharts;
using namespace std;
class MainWindow : public QWidget
{
    Q_OBJECT
public:
      explicit MainWindow(QWidget *parent = nullptr);
      ~MainWindow();
      void initUI();
      void initConect();
      void seria_port_init();//串口初始化函数声明
      void initCharts();//初始化图表
      void updateChartData(double lux,double temp,double hum);
private slots:
      void readSerialData();//串口数据读取槽函数
      void sendGetDataCommand();
      void onChartUpdateTimer();
      void updateVideoFrame(QImage img);//更新视频帧
      void handleCameraError(const QString &errorMsg);
      void checkAlarms();  // 检查告警
      void onAlarmPageTimeout();  // 告警分页定时器
      void onLoginLabelClicked();
      void onLoginSuccess();
      void showPlay();
      void showMainWindow();
protected:
       bool eventFilter(QObject *obj, QEvent *event) override;
private:
      QHBoxLayout * mainLayout;
      QVBoxLayout *leftLayout,*midLayout,*rightLayout;
      QWidget *leftTopWidget,*leftMidWidget,*leftButtonWidget;
      QWidget *midTittleWidget,*midVideoWidget,*midButtonWidget;
      QWidget *rightTopWidget,*rightMidWidget,*rightButtonWidget;
      QWidget *leftWidget,*midWidget,*rightWidget;
      QLabel *luxLabel,*humLabel,*tempLabel,*loginLabel,*oneLabel,*fourLabel;
      QVBoxLayout *leftTopLayout;
      QHBoxLayout* midButtonLayout,*midtopLayout;
      QLabel *title;

      //串口相关成员
      QSerialPort *serial;//串口对象
      QString buffer;//数据缓存区
      QLabel *luxValueLabel,*tempValueLabel,*humValueLabel;
      QTimer *dataRequestTimer;

      //开关灯控件
      QPushButton *ledControlBtn;
      QPushButton *fanControlBtn;
      bool isLedOn;
      bool isFanOn;
      void onLedControlClicked();
      void onFanControlClicked();

      //图表
      QChart *chart;
      QChartView *chartView;
      QSplineSeries *seriesLux;//光照曲线
      QSplineSeries *seriesTemp;
      QSplineSeries *seriesHum;
      QDateTimeAxis *axisX;//x轴
      QValueAxis *axisYLux;
      QValueAxis *axisYTemp;
      QValueAxis *axisYHum;
      QTimer *chartUpdateTimer;
      double currentLux,currentTemp,currentHum;

      //周温度柱状图
      QChart *weekTempChart;
      QChartView *weekTempChartView;
      QBarSeries *weekTempSeries;
      QValueAxis *weekTempAxisY;
      QCategoryAxis *weekTempAxisX;
      QTimer *weekChartUpdateTimer;

      void initWeekTempChart();
      QList<QPair<double, double>> updateWeekTempChart();
      QList<QPair<double,double>>getWeekTempData();

      //视频显示相关
      QLabel *videoLabel1,*videoLabel2,*videoLabel3,*videoLabel4;
      QLabel *videoLabel1_4;
      QGridLayout *videoGridLayout;//用与四通道布局
      QStackedLayout *videoStackLayout;//用于切换单四通道
      CameraThread *cameraThread=nullptr;
      bool isSingleChannel;
      QString currentCamera;

      //录制相关
      QTimer *captureTimer;//定时录制
      int captureInterval;//录制间隔
      QString videoSavePath;//保存视频路径

      //初始化视频显示
      void initVideoDisplay();
      //加载视频配置文件
      void loadVideoConfig();
      //开始摄像头监控
      void startCameraMonitoring();
      //切换视频通道模式
      void switchChannelMode(bool singleChannel);
      void openSettingWindow();
      void onSettingSaved();

      //优化
      QTimer *renderTimer;  // 渲染帧率控制定时器
      QImage lastFrame;    // 缓存最后一帧，避免重复渲染
      bool canRender;      // 渲染标记

      //设备列表
      void initDeviceList();
      //告警
      QTableWidget *alarmTableWidget;
      QChart *alarmPieChart;
      QPieSeries *alarmPieSeries;
      QChartView *alarmPieChartView;
      QTimer *alarmPageTimer;
      QTimer *alarmCheckTimer;
      QList<AlarmInfo> alarmList;
      int currentAlarmPage=0;


      void checkSingleAlarm(const QString &type, double value, double generalThresh, double seriousThresh);
      void saveAlarmToDatabase(const AlarmInfo &alarm);
      void handleDeviceControl(const AlarmInfo &alarm);
      void updateAlarmTable();
      void updateAlarmPieChart();

      VideoPlaybackWidget * playbackWidget;
      LoginWidget * loginWidget;


};

#endif // MAINWINDOW_H
