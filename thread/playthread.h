#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H
extern "C"
{
    #include "libavcodec/avcodec.h"            //编解码库
    #include "libavdevice/avdevice.h"          //输入输出设备库
    #include "libavfilter/avfilter.h"          //音视频滤镜库
    #include "libavformat/avformat.h"          //文件格式和协议库
    #include "libavutil/avutil.h"              //音视频处理
    #include "libswresample/swresample.h"      //音频重采样
    #include "libswscale/swscale.h"            //图像格式转换
}
#include<QString>
#include<QDebug>
#include<QImage>
#include<QList>
#include<QThread>
#include<QMutex>
#include<QMutexLocker>
#include<atomic>
#include"data/singletondb.h"

class PlayThread:public QThread
{
    Q_OBJECT
public:
    explicit PlayThread(QString videopath);
    ~PlayThread();

    int openVideo();
    int findVideoStream();
    int openDecoder();
    void prepareData();
    void decoderRGB32();
    QString getVideopath() const;
    void setVideopath(const QString &value);

    // 线程安全的getter/setter
    bool getIsPlay() const;
    void setIsPlay(bool value);

    bool getIsStop() const;
    void setIsStop(bool value);

    double getSpeedFactor() const;
    void setSpeedFactor(double factor);
    void seekTo(qint64 pos);
    void seekToFrame(int frameIndex);

    qint64 getTotalDuration() const;
    qint64 getCurrentPosition() const;
    double getFps() const { return fps; }
    int getTotalFrames() const { return totalFrames; }

private:
    QString videopath;
    AVFormatContext* pFormatContext;
    int videoIndex;
    AVCodecContext* pCodecContext;
    AVCodec* decoder;
    AVPacket* pkt;
    AVFrame* frame,*RGBFrame,*YUVFrame;
    SwsContext *pSwsContext,*pSwsContextYUV;
    uint8_t* bufferRGB,*bufferYUV;
    QList<QImage> imageList;

    // 使用 std::atomic 保证跨线程读写安全性（替代不可靠的 volatile）
    std::atomic<bool> isPlay;
    std::atomic<bool> isStop;

    // 以下变量被多个线程访问，需互斥锁保护
    mutable QMutex m_stateMutex;  // 保护 speedFactor, currentPosition, isSeeking, currentFrameCount, pendingSeek
    double speedFactor;
    int baseDelay;
    qint64 currentPosition;
    bool isSeeking;
    int totalFrames;
    double fps;
    int currentFrameCount;

    // 跨线程跳转请求：UI线程写入，解码线程读取并执行
    bool  m_hasPendingSeek;    // 是否有待处理的跳转请求
    qint64 m_pendingSeekPos;   // 待跳转目标位置（毫秒）

    int scanTotalFrames();
    void executeSeek();  // 仅解码线程内部调用，执行实际的 FFmpeg 跳转

protected:
    void run();
signals:
    void sendimg(QImage img);
    void sendPosition(qint64 pos);
    void sendFrameInfo(int currentFrame, int totalFrames, double fps);

};

#endif // PLAYTHREAD_H
