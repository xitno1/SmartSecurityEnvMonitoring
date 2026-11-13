#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H
#include<QThread>
#include"view/settingwidget.h"
#include<QDateTime>
#include<QMutex>
extern "C"
{
    #include "libavcodec/avcodec.h"            //编解码库，最重要的模块之一
    #include "libavdevice/avdevice.h"            //输入输出设备库；读取摄像头的
    #include "libavfilter/avfilter.h"            //音视频滤镜库；进行音视频处理与编辑
    #include "libavformat/avformat.h"            //文件格式和协议库
    #include "libavutil/avutil.h"                //音视频处理
    #include "libswresample/swresample.h"    //音频重采样
    #include "libswscale/swscale.h"            //图像进行格式转换
}
#include<QString>
#include<QDebug>
#include<QImage>
#include"encodetoh264.h"//编码类
#include"data/singletondb.h"
#include"data/device.h"
class CameraThread:public QThread
{
    Q_OBJECT
public:
    explicit  CameraThread(QString cameraName);
    ~CameraThread() override;
    int openCamera();//1.打开摄像头
    int findVideoStream();//2.查找视频流
    int openDecoder();//3.打开解码器
    void prepareData();//4.数据准备
    void decoderRGB32YUV420P();
    void setSavePath(const QString &path);//视频保存的路径
    void setCaptureInterval(int interval);
    int initCamera();
private:
    QString cameraName;
    AVFormatContext *pFormatContext;
    int videoIndex;
    AVCodecContext *pCodecContext;
    AVCodec *decoder;
    AVPacket *pkt;
    AVFrame*frame,*RGBFrame,*YUVFrame;
    SwsContext *pSwsContext,*pSwsContextYUV;// 转置格式的结构体指针
    uint8_t* bufferRGB,*bufferYUV;
    Encodetoh264 *encode;
    QString savePath;//存储视频保存路径
    int captureInterval; // 采集间隔（秒）
    qint64 fileStartTime; // 当前文件开始时间（毫秒）
    Device *device;
    int frameCount;  // 记录实际录制的帧数

    // 用于保护跨线程访问的成员
    mutable QMutex m_imgMutex;       // 保护RGB帧缓冲区，防止UI读取时被覆盖
protected:
    void run();
signals:
    void sendimg(QImage img);
    void errorOccurred(const QString &errorMsg);

};

#endif // CAMERATHREAD_H
