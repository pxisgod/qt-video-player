#include "Track.h"
#include "Demuxer.h"

bool Demuxer::pause_condition()
{
    return m_packet_queue0->is_full() || m_packet_queue1->is_full();
}
bool Demuxer::stop_condition(){
    return false;
}
bool Demuxer::notify_condition(){
    return !m_packet_queue0->is_full() && !m_packet_queue1->is_full();
}
long Demuxer::get_wait_time(){
    return 0;
}
void Demuxer::deal_neg_wait_time(){
}

int Demuxer::init()
{
    int result = 0;

    // 1.创建封装格式上下文
    AVFormatContext *av_format_context = avformat_alloc_context();
    m_av_format_context=std::shared_ptr<AVFormatContext>(av_format_context, [](AVFormatContext *ptr)
                                                          {
            avformat_close_input(&ptr);
            avformat_free_context(ptr); });
    // 2.打开文件
    if (avformat_open_input(&av_format_context, m_url.data(), NULL, NULL) != 0)
    {
        qDebug("DecoderBase::InitFFDecoder avformat_open_input fail.");
        return -1;
    }
    else
    {
        m_duration = av_format_context->duration / AV_TIME_BASE * 1000; // us to ms
        m_packet_queue0 = std::make_shared<PacketQueue>();
        m_packet_queue1 = std::make_shared<PacketQueue>();
        return create_track_list();
    }
}

void Demuxer::seek(long position)
{
    clean_func();
    int64_t seek_target = static_cast<int64_t>(position * 1000);//微秒
    int64_t seek_min = INT64_MIN;
    int64_t seek_max = INT64_MAX;
    avformat_seek_file(m_av_format_context.get(), -1, seek_min, seek_target, seek_max, 0);
}

int Demuxer::work_func()
{
    AVPacket *packet = av_packet_alloc();
    std::shared_ptr<AVPacket> packet_ptr = std::make_unique<AVPacket>(
        packet, [](AVPacket *ptr)
        {
                if(ptr!=nullptr)
                    av_packet_unref(ptr); });
    int result = av_read_frame(m_av_format_context.get(), packet_ptr.get());
    if (result != 0)
    {
        if (result == AVERROR_EOF)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        auto iter = m_track_map.find(packet_ptr->stream_index);
        if (iter != m_track_map.end())
        {
            auto track = iter->second;
            if (auto track_ptr = track.lock())
            {
                track_ptr->append_packet(packet_ptr);
            }
        }
    }
    return 0;
}
void Demuxer::clean_func()
{
    m_packet_queue0->clear();
    m_packet_queue1->clear();
}

int Demuxer::create_track_list()
{
    Demuxer::Ptr demuxer = std::static_pointer_cast<Demuxer>(shared_from_this());
    // 3.获取音视频流信息
    if (avformat_find_stream_info(m_av_format_context.get(), NULL) < 0)
    {
        qDebug("DecoderBase::InitFFDecoder avformat_find_stream_info fail.");
        return -1;
    }

    // 4.获取音视频流索引
    Track::Ptr track;
    for (uint32_t i = 0; i < m_av_format_context->nb_streams; i++)
    {
        switch (m_av_format_context->streams[i]->codecpar->codec_type)
        {
            /*
        case AVMEDIA_TYPE_AUDIO:
            track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_AUDIO, m_packet_queue0);
            add_thread(track);
            if (track->init() != 0) // 初始化失败
            {
                return -1;
            }
            else
            {
                m_track_list[0] = track;
                m_track_map.emplace(i, track);
            }
            break;
            */
        case AVMEDIA_TYPE_VIDEO:
            track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_VIDEO, m_packet_queue1);
            track->get_clock()->set_master_clock(m_clock);
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
    }
    return 0;
}