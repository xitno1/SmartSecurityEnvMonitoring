#ifndef TRANCECODE_H
#define TRANCECODE_H
extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavdevice/avdevice.h"
    #include "libavfilter/avfilter.h"
    #include "libavformat/avformat.h"
    #include "libavutil/avutil.h"
    #include "libswresample/swresample.h"
    #include "libswscale/swscale.h"
}
#include<QString>
#include<QDebug>
#include<QObject>
#include<QThread>

// TranceCode 本身保持为纯数据+逻辑类（非QObject），由调用方在线程中执行
// 调用方通过 TranceCodeWorker 将其放入后台线程
class TranceCode
{
public:
    TranceCode(const QString& inputPath, const QString& outputPath, const QString& format = "MP4");
    TranceCode(QString sourcePath, QString destPath);
    ~TranceCode();

    bool initDecode();
    bool initEncode();
    bool tranceCode();

    // 获取进度信息（线程安全）
    int getProgress() const { return m_progress; }
    QString getDestPath() const { return destPath; }

private:
    QString targetFormat;
    QString sourcePath, destPath;
    AVFormatContext *pFormatContext, *destFormatContext;
    AVPacket *pkt;
    int videoIndex;
    AVStream *newStream;
    int m_progress;  // 进度百分比 0-100

    AVOutputFormat* getOutputFormatByTarget();
};


// 工作线程对象：将 TranceCode 放在后台线程中执行，避免阻塞UI
class TranceCodeWorker : public QObject
{
    Q_OBJECT
public:
    explicit TranceCodeWorker(const QString &inputPath,
                              const QString &outputPath,
                              const QString &format = "MP4",
                              QObject *parent = nullptr);
    ~TranceCodeWorker();

public slots:
    void process();  // 在 worker 线程中执行转码

signals:
    void finished(bool success, const QString &outputPath);
    void progress(int percent);
    void errorOccurred(const QString &errorMsg);

private:
    QString m_inputPath;
    QString m_outputPath;
    QString m_format;
};

#endif // TRANCECODE_H
