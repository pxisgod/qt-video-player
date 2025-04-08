#include "Track.h"
#include "Demuxer.h"
#include "AudioFrameScaler.h"
#include "VideoFrameScaler.h"

Track::Track(uint32_t stream_id,std::shared_ptr<Demuxer> demuxer,AVMediaType media_type,std::shared_ptr<PacketQueue> packet_queue){
    m_stream_id=stream_id;
    m_demuxer=demuxer;
    m_media_type=media_type;
    m_packet_queue=packet_queue;
    m_clock=std::make_shared<SyncClock>(demuxer->get_av_format_context()->streams[stream_id]->time_base);
}
bool Track::pause_condition()
{
    return m_packet_queue->is_empty() || m_frame_queue->is_full();
}

bool Track::stop_condition()
{
    return m_packet_queue->is_empty();
}


long Track::get_wait_time(){
    return 0;
}
void Track::deal_neg_wait_time(){
}


int Track::init()
{
    ThreadChain::init(); //设置消息链
    // 5.获取解码器参数
    AVCodecParameters *codecParameters = m_demuxer->get_av_format_context()->streams[m_stream_id]->codecpar;

    // 6.获取解码器
    AVCodec *avCodec = const_cast<AVCodec *>(avcodec_find_decoder(codecParameters->codec_id));
    if (avCodec == nullptr)
    {
        qDebug("DecoderBase::InitFFDecoder avcodec_find_decoder fail.");
        return -1;
    }

    // 7.创建解码器上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    m_av_codec_context = std::shared_ptr<AVCodecContext>(
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
    m_frame_queue = std::make_shared<FrameQueue>();
    return create_scaler();
}

int Track::create_scaler()
{
    std::shared_ptr<Scaler> scaler;
    std::shared_ptr<Track> track=std::static_pointer_cast<Track>(shared_from_this());
    switch (m_media_type)
    {
    case AVMEDIA_TYPE_AUDIO:
        scaler = std::make_shared<AudioFrameScaler>(m_frame_queue, track);
        add_thread(scaler);
        if (scaler->init() != 0) // 初始化失败
        {
            return -1;
        }
        else
        {
            m_scaler = scaler;
        }
        break;
    case AVMEDIA_TYPE_VIDEO:
        scaler = std::make_shared<VideoFrameScaler>(m_frame_queue, track);
        add_thread(scaler);
        if (scaler->init() != 0) // 初始化失败
        {
            return -1;
        }
        else
        {
            m_scaler = scaler;
        }
        break;

    default:
        return -1;
    }
    return 0;
}
void Track::seek(long position)
{
    clean_func();
    ThreadChain::seek(position);
}
int Track::work_func()
{
    if(m_packet_queue->is_empty())
    {
        return 0;
    }
    int ret = avcodec_send_packet(m_av_codec_context.get(), m_packet_queue->read_packet().get());
    if (ret == 0)
    {
        int frame_count = 0;
        auto scaler = m_scaler.lock();
        if (!scaler)
        {
            return 0;
        }
        AVFrame *frame = av_frame_alloc();
        std::shared_ptr<AVFrame> frame_ptr(
            frame, [](AVFrame *ptr)
            {
                        if(ptr!=nullptr)
                            av_frame_unref(ptr); });
        while (avcodec_receive_frame(m_av_codec_context.get(), frame_ptr.get()) == 0)
        {
            scaler->append_frame(frame_ptr);
            frame = av_frame_alloc();
            frame_ptr=std::shared_ptr<AVFrame>(
                frame, [](AVFrame *ptr)
                {
                            if(ptr!=nullptr)
                                av_frame_unref(ptr); });
            frame_count++;         
        }
        m_packet_queue->remove_packet_3();
        m_demuxer->notify(); //通知demuxer
        return 0;
    }
    else if (ret == AVERROR_EOF)
    {
        m_packet_queue->remove_packet_3();
        return 1;
    }
    else
    {
        return -1;
    }
}
void Track::clean_func()
{
    m_frame_queue->clear();
    avcodec_flush_buffers(m_av_codec_context.get());
}

void Track::append_packet(std::shared_ptr<AVPacket> packet)
{
    m_packet_queue->append_packet(packet);
    notify();
}