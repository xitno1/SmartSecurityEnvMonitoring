#include "trancecode.h"
#include<QFileInfo>

// ===================== TranceCode 实现 =====================

TranceCode::TranceCode(const QString& inputPath, const QString& outputPath, const QString& format)
    : sourcePath(inputPath), destPath(outputPath), targetFormat(format)
{
    av_register_all();
    avformat_network_init();
    this->videoIndex = -1;
    this->pFormatContext = nullptr;
    this->destFormatContext = nullptr;
    this->pkt = nullptr;
    this->newStream = nullptr;
    this->m_progress = 0;
    qDebug() << "转码初始化 - 源路径:" << sourcePath << "目标路径:" << destPath << "目标格式:" << targetFormat;
}

TranceCode::TranceCode(QString sourcePath, QString destPath)
    : sourcePath(sourcePath), destPath(destPath), targetFormat("MP4")
{
    av_register_all();
    avformat_network_init();
    this->videoIndex = -1;
    this->pFormatContext = nullptr;
    this->destFormatContext = nullptr;
    this->pkt = nullptr;
    this->newStream = nullptr;
    this->m_progress = 0;
}

TranceCode::~TranceCode()
{
    // 确保资源在析构时释放（如果转码中途失败）
    if (pkt)
    {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
    if (destFormatContext)
    {
        avformat_free_context(destFormatContext);
        destFormatContext = nullptr;
    }
    if (pFormatContext)
    {
        avformat_close_input(&pFormatContext);
        pFormatContext = nullptr;
    }
}

bool TranceCode::initDecode()
{
    this->pFormatContext = avformat_alloc_context();
    int res = avformat_open_input(&this->pFormatContext, sourcePath.toUtf8().constData(), nullptr, nullptr);
    if (res != 0)
    {
        char errbuf[1024] = {0};
        av_strerror(res, errbuf, sizeof(errbuf));
        qDebug() << "avformat_open_input 失败:" << errbuf;
        return false;
    }
    qDebug() << "avformat_open_input 成功";

    res = avformat_find_stream_info(pFormatContext, nullptr);
    if (res < 0)
    {
        char errbuf[1024] = {0};
        av_strerror(res, errbuf, sizeof(errbuf));
        qDebug() << "avformat_find_stream_info 失败:" << errbuf;
        avformat_close_input(&pFormatContext);
        return false;
    }
    qDebug() << "avformat_find_stream_info 成功";

    for (int i = 0; i < pFormatContext->nb_streams; i++)
    {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            this->videoIndex = i;
            break;
        }
    }

    if (this->videoIndex == -1)
    {
        qDebug() << "未找到视频流";
        avformat_close_input(&pFormatContext);
        return false;
    }
    qDebug() << "找到视频流，索引:" << videoIndex;
    return true;
}

AVOutputFormat* TranceCode::getOutputFormatByTarget()
{
    AVOutputFormat* outFormat = nullptr;

    if (targetFormat.compare("MP4", Qt::CaseInsensitive) == 0) {
        outFormat = av_guess_format("mp4", nullptr, nullptr);
    }
    else if (targetFormat.compare("AVI", Qt::CaseInsensitive) == 0) {
        outFormat = av_guess_format("avi", nullptr, nullptr);
    }
    else if (targetFormat.compare("MKV", Qt::CaseInsensitive) == 0) {
        outFormat = av_guess_format("matroska", nullptr, nullptr);
    }
    else if (targetFormat.compare("FLV", Qt::CaseInsensitive) == 0) {
        outFormat = av_guess_format("flv", nullptr, nullptr);
    }
    else {
        outFormat = av_guess_format("mp4", nullptr, nullptr);
        targetFormat = "MP4";
        qDebug() << "未知格式，默认使用MP4封装";
    }

    return outFormat;
}

bool TranceCode::initEncode()
{
    this->destFormatContext = avformat_alloc_context();
    AVOutputFormat* outFormat = getOutputFormatByTarget();
    if (!outFormat) {
        qDebug() << "获取输出格式失败";
        return false;
    }
    this->destFormatContext->oformat = outFormat;

    int res = avio_open(&this->destFormatContext->pb, destPath.toUtf8().constData(), AVIO_FLAG_WRITE);
    if (res < 0)
    {
        char errbuf[1024] = {0};
        av_strerror(res, errbuf, sizeof(errbuf));
        qDebug() << "avio_open 失败:" << errbuf;
        avformat_free_context(destFormatContext);
        destFormatContext = nullptr;
        return false;
    }
    qDebug() << "avio_open 成功";

    this->newStream = avformat_new_stream(destFormatContext, nullptr);
    if (!this->newStream)
    {
        qDebug() << "avformat_new_stream 失败";
        avio_close(destFormatContext->pb);
        avformat_free_context(destFormatContext);
        destFormatContext = nullptr;
        return false;
    }
    qDebug() << "avformat_new_stream 成功";

    res = avcodec_parameters_copy(this->newStream->codecpar, this->pFormatContext->streams[videoIndex]->codecpar);
    if (res < 0)
    {
        char errbuf[1024] = {0};
        av_strerror(res, errbuf, sizeof(errbuf));
        qDebug() << "avcodec_parameters_copy 失败:" << errbuf;
        avio_close(destFormatContext->pb);
        avformat_free_context(destFormatContext);
        destFormatContext = nullptr;
        return false;
    }

    this->newStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    this->newStream->codecpar->codec_tag = 0;
    this->newStream->time_base = this->pFormatContext->streams[videoIndex]->time_base;

    qDebug() << "avcodec_parameters_copy 成功";

    res = avformat_write_header(destFormatContext, nullptr);
    if (res < 0)
    {
        char errbuf[1024] = {0};
        av_strerror(res, errbuf, sizeof(errbuf));
        qDebug() << "avformat_write_header 失败:" << errbuf;
        avio_close(destFormatContext->pb);
        avformat_free_context(destFormatContext);
        destFormatContext = nullptr;
        return false;
    }
    qDebug() << "avformat_write_header 成功";
    return true;
}

bool TranceCode::tranceCode()
{
    if (!pFormatContext || !destFormatContext || !newStream)
    {
        qDebug() << "转码初始化未完成";
        return false;
    }

    this->pkt = av_packet_alloc();
    if (!pkt)
    {
        qDebug() << "AVPacket 分配失败";
        return false;
    }

    int frameCount = 0;
    int res = 0;

    // 先扫描总帧数以计算进度
    int totalFrames = pFormatContext->streams[videoIndex]->nb_frames;
    if (totalFrames <= 0) totalFrames = 1;  // 避免除零

    while (av_read_frame(this->pFormatContext, pkt) == 0)
    {
        if (pkt->stream_index == videoIndex)
        {
            frameCount++;

            if (this->pkt->pts == AV_NOPTS_VALUE)
            {
                AVRational time_base1 = this->pFormatContext->streams[videoIndex]->time_base;
                int64_t duration = (double)AV_TIME_BASE / av_q2d(this->pFormatContext->streams[videoIndex]->r_frame_rate);
                pkt->pts = (double)frameCount * duration / double(av_q2d(time_base1) * AV_TIME_BASE);
                pkt->dts = pkt->pts;
                pkt->duration = duration / double(av_q2d(time_base1) * AV_TIME_BASE);
            }
            else if (pkt->pts < pkt->dts)
            {
                av_packet_unref(pkt);
                continue;
            }

            pkt->pts = av_rescale_q_rnd(pkt->pts,
                                       this->pFormatContext->streams[videoIndex]->time_base,
                                       this->newStream->time_base,
                                       (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
            pkt->dts = av_rescale_q_rnd(pkt->dts,
                                       this->pFormatContext->streams[videoIndex]->time_base,
                                       this->newStream->time_base,
                                       (AVRounding)(AV_ROUND_INF | AV_ROUND_PASS_MINMAX));
            pkt->duration = av_rescale_q(pkt->duration,
                                       this->pFormatContext->streams[videoIndex]->time_base,
                                       this->newStream->time_base);

            pkt->stream_index = 0;
            pkt->pos = -1;
            pkt->flags |= AV_PKT_FLAG_KEY;

            res = av_interleaved_write_frame(this->destFormatContext, pkt);
            if (res != 0)
            {
                char errbuf[1024] = {0};
                av_strerror(res, errbuf, sizeof(errbuf));
                qDebug() << "写入帧失败(" << frameCount << "):" << errbuf;
            }

            // 更新进度
            m_progress = (frameCount * 100) / totalFrames;
            if (m_progress > 100) m_progress = 100;
        }
        av_packet_unref(pkt);
    }

    av_write_trailer(this->destFormatContext);
    qDebug() << "转码完成，总帧数:" << frameCount;

    // 资源释放
    av_packet_free(&pkt);
    pkt = nullptr;
    if (destFormatContext)
    {
        avio_close(destFormatContext->pb);
        avformat_free_context(destFormatContext);
        destFormatContext = nullptr;
    }
    if (pFormatContext)
    {
        avformat_close_input(&pFormatContext);
        pFormatContext = nullptr;
    }

    m_progress = 100;

    QFileInfo fileInfo(destPath);
    if (fileInfo.exists() && fileInfo.size() > 0)
    {
        qDebug() << "目标文件生成成功，大小:" << fileInfo.size() / 1024 << "KB";
        return true;
    } else {
        qDebug() << "目标文件生成失败";
        return false;
    }
}


// ===================== TranceCodeWorker 实现 =====================

TranceCodeWorker::TranceCodeWorker(const QString &inputPath,
                                   const QString &outputPath,
                                   const QString &format,
                                   QObject *parent)
    : QObject(parent)
    , m_inputPath(inputPath)
    , m_outputPath(outputPath)
    , m_format(format)
{
}

TranceCodeWorker::~TranceCodeWorker()
{
}

void TranceCodeWorker::process()
{
    TranceCode transcoder(m_inputPath, m_outputPath, m_format);

    if (!transcoder.initDecode())
    {
        emit errorOccurred("解码初始化失败");
        emit finished(false, m_outputPath);
        return;
    }

    if (!transcoder.initEncode())
    {
        emit errorOccurred("编码初始化失败");
        emit finished(false, m_outputPath);
        return;
    }

    emit progress(0);

    bool success = transcoder.tranceCode();

    emit progress(100);
    emit finished(success, m_outputPath);
}
