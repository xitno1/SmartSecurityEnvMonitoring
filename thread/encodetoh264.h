#ifndef ENCODETOH264_H
#define ENCODETOH264_H
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

class Encodetoh264
{
public:
    Encodetoh264();
    ~Encodetoh264();
    void setWH(int w,int h);
    void prepareEncode();
    void encodeFrame(AVFrame * yuvFrame);
    void writeTailer();
    void setOutputFile(QString path);
private:
    AVFormatContext* pFormatContext;
    AVOutputFormat* outFormat;
    AVStream * newStream;
    AVCodecContext *pCodecContext;
    AVCodec * encoder;
    AVPacket* pkt;
    int pkt_index;
    int width,height;
    QString outputFilePath;
    bool m_initialized;  // 标记是否已完成prepareEncode，防止重复初始化
};

#endif // ENCODETOH264_H
