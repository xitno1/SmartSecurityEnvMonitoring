#include "encodetoh264.h"
#include<QDateTime>

Encodetoh264::Encodetoh264()
{
    av_register_all();
    this->pFormatContext = nullptr;
    this->outFormat = nullptr;
    this->newStream = nullptr;
    this->pCodecContext = nullptr;
    this->encoder = nullptr;
    this->pkt = av_packet_alloc();
    this->pkt_index = 0;
    this->outputFilePath = "";
    this->m_initialized = false;
    this->width = 0;
    this->height = 0;
}

Encodetoh264::~Encodetoh264()
{
    if (pkt)
    {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
    // 如果尚未调用 writeTailer（异常退出情况），清理残留资源
    if (m_initialized && pFormatContext)
    {
        av_write_trailer(pFormatContext);
        avio_close(pFormatContext->pb);
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
    }
}

void Encodetoh264::setOutputFile(QString path)
{
    this->outputFilePath = path;
}

void Encodetoh264::setWH(int w, int h)
{
    this->width = w;
    this->height = h;
}

void Encodetoh264::prepareEncode()
{
    // 防止连续两次 prepareEncode 而不调用 writeTailer
    if (m_initialized)
    {
        qWarning() << "Encodetoh264: 重复调用 prepareEncode，先自动清理旧资源";
        writeTailer();
    }

    pFormatContext = avformat_alloc_context();
    if (!pFormatContext)
    {
        qDebug() << "avformat_alloc_context fail";
        return;
    }

    QString filename;
    if (outputFilePath.length() > 0)
    {
        filename = outputFilePath;
    }
    else
    {
        filename = QString("../fileOut/Video/%1.h264").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    }
    qDebug() << "h264filename:" << filename;

    outFormat = av_guess_format("h264", filename.toUtf8(), nullptr);
    if (outFormat == nullptr)
    {
        qDebug() << "av_guess_format fail，尝试mpegts";
        outFormat = av_guess_format("mpegts", filename.toUtf8(), nullptr);
    }
    if (outFormat == nullptr) {
        qDebug() << "av_guess_format完全失败";
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
        return;
    }
    qDebug() << "av_guess_format success";

    pFormatContext->oformat = outFormat;

    int res = avio_open(&pFormatContext->pb, filename.toUtf8(), AVIO_FLAG_WRITE);
    if (res < 0)
    {
        qDebug() << "avio_open fail";
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
        return;
    }
    qDebug() << "avio_open success";

    newStream = avformat_new_stream(pFormatContext, nullptr);
    if (newStream == nullptr)
    {
        qDebug() << "avformat_new_stream fail";
        avio_close(pFormatContext->pb);
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
        return;
    }
    qDebug() << "avformat_new_stream success";

    pCodecContext = newStream->codec;
    pCodecContext->width = width;
    pCodecContext->height = height;
    pCodecContext->time_base = {1, 25};
    pCodecContext->bit_rate = 400000;
    pCodecContext->framerate = {25, 1};
    pCodecContext->gop_size = 10;
    pCodecContext->qmax = 51;
    pCodecContext->qmin = 10;
    pCodecContext->max_b_frames = 0;
    pCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecContext->codec_id = outFormat->video_codec;

    encoder = avcodec_find_encoder(pCodecContext->codec_id);
    if (encoder == nullptr)
    {
        qDebug() << "avcodec_find_encoder fail";
        avio_close(pFormatContext->pb);
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
        return;
    }
    qDebug() << "avcodec_find_encoder success";

    res = avcodec_open2(pCodecContext, encoder, nullptr);
    if (res != 0)
    {
        qDebug() << "avcodec_open2 encoder fail";
        avio_close(pFormatContext->pb);
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
        return;
    }
    qDebug() << "avcodec_open2 encoder success";

    res = avformat_write_header(pFormatContext, nullptr);
    if (res < 0)
    {
        qDebug() << "avformat_write_header fail";
        avio_close(pFormatContext->pb);
        avformat_free_context(pFormatContext);
        pFormatContext = nullptr;
        return;
    }
    qDebug() << "avformat_write_header success";

    m_initialized = true;
    pkt_index = 0;  // 重置包索引
}

void Encodetoh264::encodeFrame(AVFrame *yuvFrame)
{
    if (!m_initialized || !pCodecContext)
    {
        qDebug() << "Encodetoh264: 未初始化，跳过编码";
        return;
    }

    int res = avcodec_send_frame(pCodecContext, yuvFrame);
    if (res != 0)
    {
        qDebug() << "avcodec_send_frame fail";
        return;
    }

    while (res >= 0)
    {
        this->pkt->pts = this->pkt_index++;
        res = avcodec_receive_packet(pCodecContext, this->pkt);
        if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
        {
            break;
        }
        av_interleaved_write_frame(pFormatContext, this->pkt);
    }
}

void Encodetoh264::writeTailer()
{
    if (!m_initialized || !pFormatContext)
    {
        m_initialized = false;
        return;
    }

    av_write_trailer(this->pFormatContext);
    avio_close(this->pFormatContext->pb);
    avformat_free_context(this->pFormatContext);
    this->pFormatContext = nullptr;
    this->newStream = nullptr;
    this->pCodecContext = nullptr;
    this->encoder = nullptr;
    this->outFormat = nullptr;
    this->m_initialized = false;
}
