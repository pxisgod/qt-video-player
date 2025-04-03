#include "Track.h"
#include "Demuxer.h"
#include "DemuxerAudioRender.h"
#include "DemuxerVideoRender.h"

bool Track::pause_condition()
{
    return m_PacketQueue->isEmpty() || m_FrameQueue->isFull();
}

bool Track::stop_condition()
{
    return m_PacketQueue->isEmpty();
}

int Track::init()
{
    // 5.获取解码器参数
    AVCodecParameters *codecParameters = m_Demuxer->get_av_format_context()->streams[m_StreamId]->codecpar;

    // 6.获取解码器
    AVCodec *avCodec = const_cast<AVCodec *>(avcodec_find_decoder(codecParameters->codec_id));
    if (avCodec == nullptr)
    {
        qDebug("DecoderBase::InitFFDecoder avcodec_find_decoder fail.");
        return -1;
    }

    // 7.创建解码器上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    m_AVCodecContext = std::make_shared<AVCodecContext>(
        avCodecContext, [](AVCodecContext *ptr)
        {
          avcodec_close(ptr);
          avcodec_free_context(&ptr); });

    if (avcodec_parameters_to_context(avCodecContext, codecParameters) != 0)
    {
        qDebug("DecoderBase::InitFFDecoder avcodec_parameters_to_context fail.");
        return -1;
    }

    AVDictionary *pAVDictionary = nullptr;
    av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
    av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
    av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
    av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

    // 8.打开解码器
    int result = avcodec_open2(avCodecContext, avCodec, &pAVDictionary);
    if (result < 0)
    {
        qDebug("DecoderBase::InitFFDecoder avcodec_open2 fail. result=%d", result);
        return -1;
    }
    return create_scaler();
}

int Track::create_scaler()
{
    switch (m_MediaType)
    {
    case AVMEDIA_TYPE_AUDIO:
        m_Render = std::make_shared<DemuxerAudio>(i, demuxer, AVMEDIA_TYPE_AUDIO, m_packet_queue0);
        add_thread(track);
        if (track->init() != 0) // 初始化失败
        {
            return -1;
        }
        else
        {
            m_track_list[1] = track;
            m_track_map.emplace(i, track);
        }
        break;
    case AVMEDIA_TYPE_VIDEO:
        track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_VIDEO, m_packet_queue1);
        add_thread(track);
        if (track->init() != 0)
        {
            return -1;
        }
        else
        {
            m_track_list[1] = track;
            m_track_map.emplace(i, track);
        }
        break;

    default:
        break;
    }
    
    return -1;
}
void Track::seek(long position)
{
    clean_func();
}
int Track::work_func()
{
    int ret = avcodec_send_packet(m_AVCodecContext.get(), m_PacketQueue->remove_packet());
    if (ret == 0)
    {
        int frameCount = 0;
        AVFrame *frame = av_frame_alloc();
        std::unique_ptr<AVFrame> frame_ptr = std::make_unique<AVFrame>(
            frame, [](AVFrame *ptr)
            {
                        if(ptr!=nullptr)
                            av_frame_unref(ptr); });
        while (avcodec_receive_frame(m_AVCodecContext.get(), frame_ptr.get()) == 0)
        {
            m_FrameQueue->append_frame(std::move(frame_ptr));
            frame_ptr = std::make_unique<AVFrame>(
                frame, [](AVFrame *ptr)
                {
                        if(ptr!=nullptr)
                            av_frame_unref(ptr); });
        }
    }
    else if (ret == AVERROR_EOF)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}
void Track::clean_func()
{
    m_FrameQueue->clear();
    avcodec_flush_buffers(m_AVCodecContext.get());
}
void Track::append_packet(std::unique_ptr<AVPacket> packet)
{
    m_PacketQueue->append_packet(std::move(packet));
    notify();
}