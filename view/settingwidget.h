#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H
#include<QWidget>
#include<QSettings>
#include<QtSerialPort/QSerialPortInfo>
#include<QFileDialog>
#include<QMessageBox>
#include<QDir>
#include<QCameraInfo>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include<QList>

class SettingWidget: public QWidget
{
    Q_OBJECT
public:
    explicit SettingWidget(QWidget *parent = nullptr);
    ~SettingWidget();
    //检查是否已存在配置文件
    bool isConfigExists();
    void initQss();
    bool validateConfig(QString &errorMsg);
    const QString CONFIG_PATH="../config.ini";//配置文件路径
 private slots:
    //选择视频存储路径
    void on_selectVideoPathBtn();

    //确定按钮点击事件
    void on_confirmBtn();
    //取消按钮点击事件
    void on_cancelBtn();
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QSettings *m_settings=nullptr;

    //串口
    QLabel *serialPortLabel;
    QComboBox *serialPortCombo;
    //路径存储
    QLabel *videoPathLabel;
    QLineEdit *videoPathEdit;
    QPushButton * selectVideoPathBtn;
    //视频采集间隔
    QLabel *captureIntervalLabel;
    QComboBox *captureIntervalCombo;
    // 摄像头配置相关
    QLabel *cameraLabel;
    QComboBox *cameraCombo;
    //告警阈值
    QGroupBox *alarmThresholdGroup;
    QLabel *tempGeneralLabel;
    QDoubleSpinBox *tempGeneralSpin;
    QLabel *tempSeriousLabel;
    QDoubleSpinBox *tempSeriousSpin;
    QLabel *humidityGeneralLabel;
    QDoubleSpinBox *humidityGeneralSpin;
    QLabel *humiditySeriousLabel;
    QDoubleSpinBox *humiditySeriousSpin;
    QLabel *lightGeneralLabel;
    QSpinBox *lightGeneralSpin;
    QLabel *lightSeriousLabel;
    QSpinBox *lightSeriousSpin;
    // 底部按钮
    QPushButton *confirmBtn;
    QPushButton *cancelBtn;
    //布局管理控件位置
   QVBoxLayout *mainLayout;       // 主布局
   QHBoxLayout *serialLayout;     // 串口行布局
   QHBoxLayout *videoPathLayout;  // 视频路径行布局
   QHBoxLayout *intervalLayout;   // 采集间隔行布局
   QHBoxLayout *cameraLayout;     // 摄像头行布局
   QGridLayout *alarmLayout;      // 告警阈值网格布局（表格式）
   QHBoxLayout *btnLayout;        // 按钮行布局

      void initUI();
      void loadSerialPorts();
      // 加载系统可用摄像头到下拉框
      void loadCameras();
      // 从配置文件读取数据并填充UI
      void loadConfigToUI();
      // 验证配置的有效性

      // 将UI数据写入配置文件
      void saveUIToConfig();
      // 转换采集间隔（UI显示文本 -> 秒数）
      int convertCaptureIntervalToSec(const QString &text);
      // 转换采集间隔（秒数 -> UI显示文本）
      QString convertSecToCaptureInterval(int sec);

signals:
      void configSaved();
};

#endif // SETTINGWIDGET_H
