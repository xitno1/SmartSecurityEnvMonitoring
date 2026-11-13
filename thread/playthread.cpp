#include "playthread.h"

PlayThread::PlayThread(QString videopath)
{
    speedFactor = 1.0;
    baseDelay = 40;
    currentPosition = 0;
    isSeeking = false;
    m_hasPendingSeek = false;
    m_pendingSeekPos = 0;
    this->isPlay = true;
    this->isStop = false;
    this->videopath = videopath;
    this->videoIndex = -1;

    pFormatContext = nullptr;
    pCodecContext = nullptr;
    decoder = nullptr;
    pkt = nullptr;
    frame = nullptr;
    RGBFrame = nullptr;
    YUVFrame = nullptr;
    bufferRGB = nullptr;
    bufferYUV = nullptr;
    pSwsContext = nullptr;
    pSwsContextYUV = nullptr;

    totalFrames = 0;
    fps = 25.0;
    currentFrameCount = 0;

    qDebug() << "PlayThread构造完成（主线程），视频路径:" << videopath;
}

// 获取视频总时长（毫秒）
qint64 PlayThread::getTotalDuration() const
{
    if (pFormatContext)
    {
        return (qint64)(pFormatContext->duration / AV_TIME_BASE * 1000);
    }
    return 0;
}

// 获取当前播放位置（毫秒）—— 线程安全
qint64 PlayThread::getCurrentPosition() const
{
    QMutexLocker locker(&m_stateMutex);
    return currentPosition;
}

// 跳转请求（由UI线程调用）—— 只设置标记，不解码线程上下文
void PlayThread::seekTo(qint64 pos)
{
    if (pos < 0) return;
    QMutexLocker locker(&m_stateMutex);
    m_pendingSeekPos = pos;
    m_hasPendingSeek = true;
}

// 按帧数跳转请求（由UI线程调用）
void PlayThread::seekToFrame(int frameIndex)
{
    if (frameIndex < 0) return;
    qint64 pos = (qint64)(frameIndex / fps * 1000.0);
    QMutexLocker locker(&m_stateMutex);
    m_pendingSeekPos = pos;
    m_hasPendingSeek = true;
}

// 在工作线程中执行实际的FFmpeg跳转操作（仅解码线程调用，线程安全）
void PlayThread::executeSeek()
{
    qint64 pos;
    {
        QMutexLocker locker(&m_stateMutex);
        if (!m_hasPendingSeek) return;
        pos = m_pendingSeekPos;
        m_hasPendingSeek = false;
    }

    if (!pFormatContext || pos < 0) return;

    isSeeking = true;

    int64_t targetPos = pos * 1000;  // 毫秒 → 微秒
    int ret = av_seek_frame(pFormatContext, videoIndex, targetPos, AVSEEK_FLAG_BACKWARD);

    if (ret < 0)
    {
        // 时间戳跳转失败（裸H264流），尝试字节级跳转
        int64_t fileSize = avio_size(pFormatContext->pb);
        if (fileSize > 0 && totalFrames > 0)
        {
            int targetFrame = (int)(pos * fps / 1000.0);
            int64_t bytePos = (int64_t)((double)targetFrame / totalFrames * fileSize);
            if (bytePos >= fileSize) bytePos = fileSize - 1024;
            if (bytePos < 0) bytePos = 0;
            ret = av_seek_frame(pFormatContext, -1, bytePos, AVSEEK_FLAG_BYTE);
        }
    }

    if (ret >= 0)
    {
        avcodec_flush_buffers(pCodecContext);
        currentPosition = pos;
        currentFrameCount = (int)(pos * fps / 1000.0);
    }

    isSeeking = false;
}

int PlayThread::openVideo()
{
    this->pFormatContext = avformat_alloc_context();
    int res = avformat_open_input(&this->pFormatContext, this->videopath.toUtf8(), nullptr, nullptr);
    if (res != 0) {
        qDebug() << "avformat_open_input fail";
    }
    return res;
}

int PlayThread::findVideoStream()
{
    int res = avformat_find_stream_info(this->pFormatContext, nullptr);
    if (res < 0) {
        qDebug() << "avformat_find_stream_info fail";
        return -1;
    }
    qDebug() << "avformat_find_stream_info success";

    for (int i = 0; i < this->pFormatContext->nb_streams; i++) {
        if (this->pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->videoIndex = i;
            break;
        }
    }
    if (this->videoIndex == -1) {
        qDebug() << "avformat_find_video_stream_info fail";
        return -1;
    }
    qDebug() << "avformat_find_video_stream_info success";
    return 0;
}

int PlayThread::scanTotalFrames() {
    int dbFrames = SingletonDB::getInstance()->getVideoFrameCount(videopath);
    if (dbFrames > 0) {
        return dbFrames;
    }

    AVFormatContext *fmtCtx = avformat_alloc_context();
    if (avformat_open_input(&fmtCtx, videopath.toUtf8(), nullptr, nullptr) != 0) {
        return 0;
    }
    avformat_find_stream_info(fmtCtx, nullptr);
    int videoIdx = -1;
    for (int i = 0; i < fmtCtx->nb_streams; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIdx = i;
            break;
        }
    }
    int total = 0;
    if (videoIdx != -1) {
        total = fmtCtx->streams[videoIdx]->nb_frames;
    }
    avformat_close_input(&fmtCtx);
    return total;
}

int PlayThread::openDecoder()
{
    this->pCodecContext = this->pFormatContext->streams[this->videoIndex]->codec;
    this->decoder = avcodec_find_decoder(this->pCodecContext->codec_id);
    if (this->decoder == nullptr) {
        qDebug() << "avcodec_find_decoder fail";
        return -1;
    }
    qDebug() << "avcodec_find_decoder success";

    int res = avcodec_open2(this->pCodecContext, this->decoder, nullptr);
    if (res != 0) {
        qDebug() << "avcodec_open2 fail";
        return -1;
    }
    qDebug() << "avcodec_open2 success";

    AVRational rate = pFormatContext->streams[videoIndex]->r_frame_rate;
    fps = av_q2d(rate);
    if (fps <= 0) fps = 25.0;

    totalFrames = scanTotalFrames();
    qDebug() << "视频总帧数:" << totalFrames << "帧率:" << fps;
    return 0;
}

void PlayThread::prepareData()
{
    qDebug() << "开始准备数据...";

    this->pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "AVPacket分配失败";
        return;
    }
    qDebug() << "AVPacket分配成功";

    this->frame = av_frame_alloc();
    this->RGBFrame = av_frame_alloc();

    if (!frame || !RGBFrame) {
        qDebug() << "Frame分配失败";
        return;
    }
    qDebug() << "Frame分配成功";

    RGBFrame->width = pCodecContext->width;
    RGBFrame->height = pCodecContext->height;
    RGBFrame->format = AV_PIX_FMT_RGB32;

    int ret = av_frame_get_buffer(RGBFrame, 0);
    if (ret < 0) {
        qDebug() << "RGB Frame缓冲区分配失败:" << ret;
        return;
    }
    qDebug() << "RGB Frame缓冲区分配成功";

    pSwsContext = sws_getContext(pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt,
                                pCodecContext->width, pCodecContext->height, AV_PIX_FMT_RGB32,
                                SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!pSwsContext) {
        qDebug() << "SWS上下文创建失败";
        return;
    }
    qDebug() << "SWS上下文创建成功";
    qDebug() << "数据准备完成，视频尺寸:" << pCodecContext->width << "x" << pCodecContext->height;
}

void PlayThread::decoderRGB32()
{
    qDebug() << "开始解码视频:" << videopath;
    this->isPlay = true;
    this->isStop = false;
    currentFrameCount = 0;
    int ret = 0;

    qint64 lastValidPosition = 0;

    if (!pFormatContext) {
        qDebug() << "错误: pFormatContext 未初始化";
        return;
    }
    if (!pCodecContext) {
        qDebug() << "错误: pCodecContext 未初始化";
        return;
    }
    if (!pSwsContext) {
        qDebug() << "错误: pSwsContext 未初始化";
        return;
    }

    qDebug() << "视频信息 - 宽度:" << pCodecContext->width << "高度:" << pCodecContext->height;
    qDebug() << "视频格式:" << pCodecContext->pix_fmt;

    while (!isStop.load())
    {
        // 检查并执行跳转请求（UI线程设置标记，解码线程自己调用FFmpeg，无竞态）
        executeSeek();

        if (!isPlay.load())
        {
            msleep(50);
            continue;
        }

        ret = av_read_frame(pFormatContext, pkt);
        if (ret != 0) {
            qDebug() << "读取帧结束或错误:" << ret;
            break;
        }

        if (pkt->stream_index == videoIndex) {
            int got_picture_ptr = 0;
            ret = avcodec_decode_video2(pCodecContext, frame, &got_picture_ptr, pkt);
            if (ret < 0) {
                qDebug() << "解码失败:" << ret;
                av_packet_unref(pkt);
                continue;
            }

            if (got_picture_ptr) {
                ret = sws_scale(pSwsContext,
                               frame->data, frame->linesize, 0, frame->height,
                               RGBFrame->data, RGBFrame->linesize);
                if (ret <= 0) {
                    qDebug() << "图像转换失败:" << ret;
                    av_packet_unref(pkt);
                    continue;
                }

                // ---- 关键修复：深拷贝 QImage，避免缓冲区竞态 ----
                // RGBFrame->data[0] 在下一轮循环会被覆盖，必须拷贝后发送
                QImage safeCopy = QImage(RGBFrame->data[0],
                                         RGBFrame->width,
                                         RGBFrame->height,
                                         QImage::Format_RGB32).copy();  // 深拷贝
                emit sendimg(safeCopy);

                // 更新时间戳
                qint64 newPosition;
                if (frame->pts != AV_NOPTS_VALUE) {
                    AVRational time_base = pFormatContext->streams[videoIndex]->time_base;
                    newPosition = (qint64)(frame->pts * av_q2d(time_base) * 1000);
                } else if (pkt->pts != AV_NOPTS_VALUE) {
                    AVRational time_base = pFormatContext->streams[videoIndex]->time_base;
                    newPosition = (qint64)(pkt->pts * av_q2d(time_base) * 1000);
                } else {
                    newPosition = (qint64)((currentFrameCount / fps) * 1000);
                }

                {
                    QMutexLocker locker(&m_stateMutex);
                    currentPosition = newPosition;
                    currentFrameCount++;
                }

                if (newPosition > lastValidPosition) {
                    lastValidPosition = newPosition;
                }

                emit sendPosition(newPosition);
                emit sendFrameInfo(currentFrameCount, totalFrames, fps);

                int actualDelay = (int)(baseDelay / speedFactor);
                msleep(actualDelay);
            }
            av_packet_unref(pkt);
        } else {
            av_packet_unref(pkt);
        }
    }

    qDebug() << "解码完成，总帧数:" << currentFrameCount
             << "最后有效位置:" << lastValidPosition << "ms";

    if (lastValidPosition > 0) {
        emit sendPosition(lastValidPosition);
        emit sendFrameInfo(totalFrames, totalFrames, fps);
    } else if (currentFrameCount > 0) {
        qint64 estimatedTotal = (qint64)((currentFrameCount / fps) * 1000);
        emit sendPosition(estimatedTotal);
        emit sendFrameInfo(currentFrameCount, currentFrameCount, fps);
    }

    emit sendPosition(getTotalDuration());
    emit sendFrameInfo(totalFrames, totalFrames, fps);

    // 注意：FFmpeg 资源统一在析构函数中释放
    // 此处不做任何释放，避免与析构函数冲突
}

QString PlayThread::getVideopath() const { return videopath; }
void PlayThread::setVideopath(const QString &value) { videopath = value; }

bool PlayThread::getIsPlay() const { return isPlay.load(); }
void PlayThread::setIsPlay(bool value) { isPlay.store(value); }

bool PlayThread::getIsStop() const { return isStop.load(); }
void PlayThread::setIsStop(bool value) { isStop.store(value); }

double PlayThread::getSpeedFactor() const
{
    QMutexLocker locker(&m_stateMutex);
    return speedFactor;
}

void PlayThread::setSpeedFactor(double factor)
{
    QMutexLocker locker(&m_stateMutex);
    speedFactor = factor;
}

void PlayThread::run()
{
    qDebug() << "PlayThread子线程启动，开始初始化FFmpeg";

    av_register_all();
    avdevice_register_all();

    if (openVideo() != 0) {
        qDebug() << "打开视频失败，子线程退出";
        return;
    }

    if (findVideoStream() != 0) {
        qDebug() << "查找视频流失败，子线程退出";
        return;
    }

    if (openDecoder() != 0) {
        qDebug() << "打开解码器失败，子线程退出";
        return;
    }

    prepareData();
    decoderRGB32();
    // FFmpeg 资源由析构函数统一释放
}

PlayThread::~PlayThread()
{
    // 设置停止标志，通知工作线程退出
    isStop.store(true);
    isPlay.store(false);

    // 等待线程正常退出（给充足时间让FFmpeg自然结束）
    if (!wait(5000))
    {
        qWarning() << "PlayThread 未能正常退出，强制终止";
        terminate();
        wait(1000);
    }

    // 线程已退出，安全释放所有FFmpeg资源
    if (pFormatContext) {
        avformat_close_input(&pFormatContext);
        pFormatContext = nullptr;
    }
    if (pkt) {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (RGBFrame) {
        av_frame_free(&RGBFrame);
        RGBFrame = nullptr;
    }
    if (YUVFrame) {
        av_frame_free(&YUVFrame);
        YUVFrame = nullptr;
    }
    if (bufferRGB) {
        av_free(bufferRGB);
        bufferRGB = nullptr;
    }
    if (bufferYUV) {
        av_free(bufferYUV);
        bufferYUV = nullptr;
    }
    if (pSwsContext) {
        sws_freeContext(pSwsContext);
        pSwsContext = nullptr;
    }
    if (pSwsContextYUV) {
        sws_freeContext(pSwsContextYUV);
        pSwsContextYUV = nullptr;
    }
    if (pCodecContext) {
        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = nullptr;
    }
}
