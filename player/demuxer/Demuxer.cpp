#include "Track.h"
#include "Demuxer.h"

int Demuxer::init_0(std::string &&url)
{
    std::lock_guard<std::mutex> lock(m_play_mutex);
    if (m_play_state == STOPPED)
    {
        long system_time = get_system_current_time();
        m_play_state = INITED;
        m_url = std::move(url);
        if (init(system_time) != 0)
        {
            uninit_0(); // 全部去除初始化
            return -1;
        }
        return 0;
    }

    return -1;
}

void Demuxer::start_0()
{
    std::lock_guard<std::mutex> lock(m_play_mutex);
    if (m_play_state == INITED)
    {
        m_play_state = PLAYING; // 默认初始状态
        // 从子向父遍历
        auto leaf_list = get_all_thread(shared_from_this());
        leaf_list.sort();
        for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
        {
            (*iter)->start();
            (*iter)->uninit(); // 线程中保存了强引用，可以清除原来的强引用
        }
        emit_event(ThreadMsg{SWITCH_PLAYING, 0});
    }
}

int Demuxer::play_0() 
{
    std::lock_guard<std::mutex> lock(m_play_mutex);
    if (m_play_state == PAUSE)
    {
        long system_time = get_system_current_time();
        m_play_state = PLAYING;
        // 从子向父遍历
        auto leaf_list = get_all_thread(shared_from_this());
        leaf_list.sort();
        for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
        {
            (*iter)->play(system_time);
        }
        emit_event(ThreadMsg{SWITCH_PLAYING, 0});
        return 0;
    }
    return -1;
}

int Demuxer::pause_0() 
{
    std::lock_guard<std::mutex> lock(m_play_mutex);
    if (m_play_state == PLAYING)
    {
        m_play_state = PAUSE;
        // 从子向父遍历
        auto leaf_list = get_all_thread(shared_from_this());
        leaf_list.sort();
        for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
        {
            (*iter)->pause();
        }
        emit_event(ThreadMsg{SWITCH_PAUSE, 0});
        return 0;
    }
    return -1;
}

int Demuxer::seek_0(long pts_time)
{
    std::lock_guard<std::mutex> lock(m_play_mutex);

    if (m_play_state == PLAYING || m_play_state == PAUSE)
    {
        // 从子向父遍历
        auto leaf_list = get_all_thread(shared_from_this());
        leaf_list.sort();
        auto list_copy = leaf_list;
        seek_0_l(leaf_list, list_copy, pts_time);
    }

    return 0;
}

void Demuxer::seek_0_l(std::list<ThreadChain::S_Ptr> &leaf_list, std::list<ThreadChain::S_Ptr> &list_copy, long pts_time)
{
    if (!leaf_list.empty())
    {
        ThreadChain::S_Ptr thread = leaf_list.back();
        std::lock_guard<std::mutex> lock(thread->m_rsc_mutex); // 反向获取所有资源锁，全部获取后才会开始调用seek方法
        leaf_list.pop_back();
        seek_0_l(leaf_list, list_copy, pts_time);
    }
    else
    {
        long system_time = get_system_current_time();
        seek_1_l(list_copy, pts_time,system_time);
    }
}

void Demuxer::seek_1_l(std::list<ThreadChain::S_Ptr> &list_copy, long pts_time,long system_time)
{
    if (!list_copy.empty())
    {
        ThreadChain::S_Ptr thread = list_copy.back();
        list_copy.pop_back();
        thread->seek(pts_time,system_time);
        seek_1_l(list_copy, pts_time,system_time);
    }
}

void Demuxer::uninit_0() // 只有顶层可以使用
{
    m_play_state = STOPPED;
    // 从子向父遍历
    auto leaf_list = get_all_thread(shared_from_this());
    leaf_list.sort();
    for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
    {
        (*iter)->uninit(); // 线程中保存了强引用，可以清除原来的强引用
    }
}

int Demuxer::stop_0()
{
    std::lock_guard<std::mutex> lock(m_play_mutex);
    if (m_play_state == PAUSE || m_play_state == PLAYING)
    {
        m_play_state = STOPPED;
        // 从子向父遍历
        auto leaf_list = get_all_thread(shared_from_this());
        leaf_list.sort();
        for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
        {
            (*iter)->stop();
        }
        emit_event(ThreadMsg{SWITCH_STOP, 0});
        return 0;
    }
    return -1;
}

bool Demuxer::pause_condition(int work_state)
{
    return m_packet_queue0->is_full() || m_packet_queue1->is_full();
}

int Demuxer::do_init(long system_time)
{
    m_clock->set_clock(0, system_time); // 设置时钟

    // 1.创建封装格式上下文
    AVFormatContext *av_format_context = avformat_alloc_context();
    m_av_format_context = std::shared_ptr<AVFormatContext>(av_format_context, [](AVFormatContext *ptr)
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
        return create_track_list(system_time);
    }
}

void Demuxer::do_seek(long pts_time,long system_time)
{
    //清理队列
    m_packet_queue0->clear();
    m_packet_queue1->clear();
    //设置文件读取位置
    int64_t seek_target = static_cast<int64_t>(pts_time * 1000000); // 微秒
    int64_t seek_min = INT64_MIN;
    int64_t seek_max = INT64_MAX;
    int ret = avformat_seek_file(m_av_format_context.get(), -1, seek_min, seek_target, seek_max, 0);
}


int Demuxer::work_func()
{
    AVPacket *packet = av_packet_alloc();
    std::shared_ptr<AVPacket> packet_ptr = std::shared_ptr<AVPacket>(
        packet, [](AVPacket *ptr)
        {
                if(ptr!=nullptr)
                    av_packet_unref(ptr); });
    std::lock_guard<std::mutex> lock(m_rsc_mutex); //获取资源锁
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
    {
        std::lock_guard<std::mutex> lock(m_rsc_mutex);
        m_packet_queue0->clear();
        m_packet_queue1->clear();
    }
    std::lock_guard<std::mutex> lock(m_play_mutex);
    if (m_play_state != STOPPED)
    {
        m_play_state = STOPPED;
        emit_event(ThreadMsg{SWITCH_STOP, 0});
    }
}



int Demuxer::create_track_list(long system_time)
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

        case AVMEDIA_TYPE_AUDIO:
            track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_AUDIO, m_packet_queue0);
            track->get_clock()->set_master_clock(m_clock);
            add_thread(track);
            if (track->init(system_time) != 0) // 初始化失败
            {
                return -1;
            }
            else
            {
                m_track_list[0] = track;
                m_track_map.emplace(i, track);
            }
            break;

        case AVMEDIA_TYPE_VIDEO:
            track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_VIDEO, m_packet_queue1);
            track->get_clock()->set_master_clock(m_clock);
            add_thread(track);
            if (track->init(system_time) != 0)
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
    emit_event(ThreadMsg{INIT_DONE, m_duration}); // 发送初始化完成事件
    return 0;
}