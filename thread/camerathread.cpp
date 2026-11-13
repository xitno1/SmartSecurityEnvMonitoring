#include "camerathread.h"
#include<QSettings>  // 用于读取配置
#include<QDir>       //用于创建目录

CameraThread::CameraThread(QString cameraName)
{
    this->encode = new Encodetoh264;
    this->cameraName = cameraName;
    this->videoIndex = -1;
    this->pkt = nullptr;
    this->pFormatContext = nullptr;
    this->pCodecContext = nullptr;
    this->frame = nullptr;
    this->RGBFrame = nullptr;
    this->YUVFrame = nullptr;
    this->bufferRGB = nullptr;
    this->bufferYUV = nullptr;
    this->pSwsContext = nullptr;
    this->pSwsContextYUV = nullptr;
    this->decoder = nullptr;

    // 读取保存路径
    QSettings settings("../config.ini", QSettings::IniFormat);
    this->savePath = settings.value("VideoConfig/VideoSavePath", "../fileOut/Video").toString();
    this->captureInterval = settings.value("VideoConfig/CaptureInterval", 30).toInt();
}

int CameraThread::initCamera()
{
    // 1. 初始化FFmpeg
    av_register_all();
    avdevice_register_all();

    // 2. 创建保存目录
    QDir dir(savePath);
    if (!dir.exists())
    {
        if (!dir.mkpath(savePath))
        {
            emit errorOccurred("视频保存目录创建失败");
            return -1;
        }
    }

    // 3. 依次执行摄像头打开、流查找、解码器打开
    int res = openCamera();
    if (res != 0) return res;

    res = findVideoStream();
    if (res != 0) return res;

    res = openDecoder();
    if (res != 0) return res;

    prepareData();
    return 0;
}


CameraThread::~CameraThread()
{
    // 请求中断并等待线程正常退出（给够时间让FFmpeg清理）
    requestInterruption();
    if (!wait(5000))  // 等待最多5秒，给FFmpeg足够时间释放资源
    {
        qWarning() << "CameraThread 未能正常退出，强制终止";
        terminate();
        wait(1000);
    }

    // 线程已完全退出，安全释放所有资源
    if (encode)
    {
        delete encode;
        encode = nullptr;
    }

    if (pkt)
    {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
    if (frame)
    {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (RGBFrame)
    {
        av_frame_free(&RGBFrame);
        RGBFrame = nullptr;
    }
    if (YUVFrame)
    {
        av_frame_free(&YUVFrame);
        YUVFrame = nullptr;
    }
    if (bufferRGB)
    {
        av_free(bufferRGB);
        bufferRGB = nullptr;
    }
    if (bufferYUV)
    {
        av_free(bufferYUV);
        bufferYUV = nullptr;
    }
    if (pSwsContext)
    {
        sws_freeContext(pSwsContext);
        pSwsContext = nullptr;
    }
    if (pSwsContextYUV)
    {
        sws_freeContext(pSwsContextYUV);
        pSwsContextYUV = nullptr;
    }
    if (pCodecContext)
    {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = nullptr;
    }
    if (pFormatContext)
    {
        avformat_close_input(&pFormatContext);
        pFormatContext = nullptr;
    }
}


int CameraThread::openCamera()
{
    this->pFormatContext = avformat_alloc_context();
    AVInputFormat *fmt = av_find_input_format("dshow");
    if (!fmt)
    {
        QString err = "FFmpeg不支持dshow设备（请检查FFmpeg编译选项）";
        emit errorOccurred(err);
        return -1;
    }

    AVDictionary *options = nullptr;

    av_dict_set(&options, "rtbufsize", "100M", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);

    QString cameraInput = QString("video=%1").arg(this->cameraName);
    qDebug() << "尝试打开摄像头：" << cameraInput;

    int res = avformat_open_input(&pFormatContext, cameraInput.toUtf8().constData(), fmt, &options);
    av_dict_free(&options);

    if (res != 0)
    {
        char errbuf[1024] = {0};
        av_strerror(res, errbuf, sizeof(errbuf));
        QString err = QString("摄像头打开失败：%1（设备名：%2）").arg(errbuf).arg(cameraName);
        qDebug() << err;
        emit errorOccurred(err);
        return -1;
    }
    qDebug() << "摄像头打开成功";
    return 0;
}

int CameraThread::findVideoStream()
{
    int res=avformat_find_stream_info(this->pFormatContext,nullptr);
    if(res<0)
    {
        QString err = "avformat_find_stream_info失败";
        qDebug() << err;
        emit errorOccurred(err);
        return -1;
    }
    qDebug()<<"avformat_find_stream_info success";

    for(int i=0;i<this->pFormatContext->nb_streams;i++)
    {
        if(this->pFormatContext->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            this->videoIndex=i;
            break;
        }
    }
    if(this->videoIndex==-1)
    {
        QString err = "未找到视频流";
        qDebug() << err;
        emit errorOccurred(err);
        return -1;
    }
    qDebug()<<"avformat_find_video_stream_info success";
    return 0;
}

int CameraThread::openDecoder()
{
    this->pCodecContext=this->pFormatContext->streams[this->videoIndex]->codec;
    this->decoder=avcodec_find_decoder(this->pCodecContext->codec_id);
    if(this->decoder==nullptr)
    {
        QString err = "avcodec_find_decoder失败";
       qDebug() << err;
       emit errorOccurred(err);
        return -1;
    }
    qDebug()<<"avcodec_find_decoder success";
    int res=avcodec_open2(this->pCodecContext,this->decoder,nullptr);
    if(res!=0)
    {
        QString err = "avcodec_open2失败";
         qDebug() << err;
         emit errorOccurred(err);
        return -1;
    }
    qDebug()<<"avcodec_open2 success";
    return 0;
}

void CameraThread::prepareData()
{
    this->pkt = av_packet_alloc();

   this->frame=av_frame_alloc();
   this->frame->width=this->pCodecContext->width;
   this->frame->height=this->pCodecContext->height;
   this->frame->format=this->pCodecContext->pix_fmt;
   this->RGBFrame=av_frame_alloc();
   this->RGBFrame->width=this->pCodecContext->width;
   this->RGBFrame->height=this->pCodecContext->height;
   this->RGBFrame->format=this->pCodecContext->pix_fmt;
   int numbyteGRB=avpicture_get_size(AV_PIX_FMT_RGB32,this->pCodecContext->width,this->pCodecContext->height);
   bufferRGB=(uint8_t*)av_malloc(numbyteGRB*sizeof (uint8_t));
   avpicture_fill((AVPicture*)this->RGBFrame,bufferRGB,AV_PIX_FMT_RGB32,
                  this->pCodecContext->width,this->pCodecContext->height);
   pSwsContext=sws_getContext(this->pCodecContext->width,this->pCodecContext->height,this->pCodecContext->pix_fmt,
                             this->pCodecContext->width,this->pCodecContext->height,AV_PIX_FMT_RGB32,
                              SWS_BICUBIC,nullptr,nullptr,nullptr);
   if(pSwsContext==nullptr)
   {
       qDebug()<<"sws_getContext fail";
       return;
   }
   qDebug()<<"sws_getContext success"<<this->pCodecContext->width;
   qDebug()<<"sws_getContext success"<<this->pCodecContext->height;

   this->YUVFrame = av_frame_alloc();
   this->YUVFrame->width=this->pCodecContext->width;
   this->YUVFrame->height=this->pCodecContext->height;
   this->YUVFrame->format=this->pCodecContext->pix_fmt;
   int numbyteYUV=avpicture_get_size(AV_PIX_FMT_YUV420P,this->pCodecContext->width,this->pCodecContext->height);
   bufferYUV=(uint8_t*)av_malloc(numbyteYUV*sizeof (uint8_t));
   avpicture_fill((AVPicture*)this->YUVFrame,bufferYUV,AV_PIX_FMT_YUV420P,
                  this->pCodecContext->width,this->pCodecContext->height);
   pSwsContextYUV=sws_getContext(this->pCodecContext->width,this->pCodecContext->height,this->pCodecContext->pix_fmt,
                             this->pCodecContext->width,this->pCodecContext->height,AV_PIX_FMT_YUV420P,
                              SWS_BICUBIC,nullptr,nullptr,nullptr);
}

void CameraThread::decoderRGB32YUV420P()
{
    if (savePath.isEmpty() || captureInterval <= 0)
    {
        qDebug() << "保存路径或采集间隔未配置";
        return;
    }

    int deviceId = 1;
    this->encode->setWH(this->pCodecContext->width, this->pCodecContext->height);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("%1.h264").arg(timestamp);
    QString fullPath = QString("%1/%2").arg(savePath).arg(filename);
    this->encode->setOutputFile(fullPath.toUtf8().constData());
    this->encode->prepareEncode();
    this->fileStartTime = QDateTime::currentMSecsSinceEpoch();
    int got_picture_ptr = -1;
    int frameCount = 0;
    bool isFirstFrame = true;
    QString currentCoverPath;

    // 帧率控制：每3帧发送1帧到UI，减轻UI线程负担
    int frameSkipCounter = 0;
    const int kFrameSkipInterval = 3;

    while (av_read_frame(this->pFormatContext, this->pkt) == 0)
    {
        if (this->isInterruptionRequested())
        {
            break;
        }

        if (this->pkt->stream_index == this->videoIndex)
        {
            avcodec_decode_video2(this->pCodecContext, this->frame, &got_picture_ptr, this->pkt);
            if (got_picture_ptr != 0)
            {
                sws_scale(pSwsContext,
                          this->frame->data, this->frame->linesize, 0, this->frame->height,
                          this->RGBFrame->data, this->RGBFrame->linesize);
                sws_scale(pSwsContextYUV,
                          this->frame->data, this->frame->linesize, 0, this->frame->height,
                          this->YUVFrame->data, this->YUVFrame->linesize);

                this->encode->encodeFrame(this->YUVFrame);
                frameCount++;

                if (isFirstFrame)
                {
                    currentCoverPath = fullPath;
                    currentCoverPath.replace(".h264", ".jpg");
                    QImage coverImg((uchar*)this->RGBFrame->data[0],
                                   this->pCodecContext->width,
                                   this->pCodecContext->height,
                                   QImage::Format_RGB32);
                    if (coverImg.save(currentCoverPath)) {
                        qDebug() << "封面保存成功：" << currentCoverPath;
                    } else {
                        qDebug() << "封面保存失败：" << currentCoverPath;
                        currentCoverPath = "";
                    }
                    isFirstFrame = false;
                }

                // ---- 关键修复：深拷贝 QImage，避免与FFmpeg缓冲区竞态 ----
                // RGBFrame->data[0] 会在下一帧被 sws_scale 覆盖
                // 必须在发送信号前做深拷贝，否则UI端读取时数据已被破坏
                frameSkipCounter++;
                if (frameSkipCounter % kFrameSkipInterval == 0)
                {
                    QImage safeCopy = QImage((uchar*)this->RGBFrame->data[0],
                                             this->pCodecContext->width,
                                             this->pCodecContext->height,
                                             QImage::Format_RGB32).copy();  // 深拷贝
                    emit sendimg(safeCopy);
                }

                qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                if (currentTime - fileStartTime >= captureInterval * 1000)
                {
                    this->encode->writeTailer();
                    QString recordTime = QDateTime::fromMSecsSinceEpoch(fileStartTime).toString("yyyy-MM-dd HH:mm:ss");
                    SingletonDB::getInstance()->insertVideo(
                        deviceId,
                        filename,
                        fullPath,
                        recordTime,
                        currentCoverPath,
                        frameCount
                    );

                    timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
                    filename = QString("%1.h264").arg(timestamp);
                    fullPath = QString("%1/%2").arg(savePath).arg(filename);
                    this->encode->setOutputFile(fullPath.toUtf8().constData());
                    this->encode->prepareEncode();
                    this->fileStartTime = currentTime;
                    frameCount = 0;
                    isFirstFrame = true;
                }
            }
        }
        av_packet_unref(this->pkt);
    }

    // 处理最后一个文件
    this->encode->writeTailer();
    if (frameCount > 0)
    {
        QString recordTime = QDateTime::fromMSecsSinceEpoch(fileStartTime).toString("yyyy-MM-dd HH:mm:ss");
        SingletonDB::getInstance()->insertVideo(
            deviceId,
            filename,
            fullPath,
            recordTime,
            currentCoverPath,
            frameCount
        );
    }
}

void CameraThread::setSavePath(const QString &path)
{
    this->savePath = path;
    QDir dir(savePath);
    if (!dir.exists())
    {
        dir.mkpath(savePath);
    }
}

void CameraThread::setCaptureInterval(int interval)
{
    captureInterval = interval;
}


void CameraThread::run()
{
    int initRes = initCamera();
    if (initRes != 0)
    {
        qDebug() << "摄像头初始化失败，线程退出";
        return;
    }

    decoderRGB32YUV420P();
    // 注意：FFmpeg资源的释放统一在析构函数中进行
    // 析构函数保证在线程完全退出后才释放，避免竞态
}
