#include "Track.h"
#include "Demuxer.h"

int Demuxer::start()
{
    int result = 0;

    // 1.创建封装格式上下文
    AVFormatContext *av_format_context = avformat_alloc_context();
    m_AVFormatContext = std::make_shared<AVFormatContext>(av_format_context, [](AVFormatContext *ptr){
            avformat_close_input(&ptr);
            avformat_free_context(ptr); });
    // 2.打开文件
    if (avformat_open_input(&av_format_context, m_Url.data(), NULL, NULL) != 0)
    {
        qDebug("DecoderBase::InitFFDecoder avformat_open_input fail.");
        return -1;
    }
    else
    {
        m_Duration = av_format_context->duration / AV_TIME_BASE * 1000; // us to ms
        m_PacketQueue0 = std::make_shared<PacketQueue>();
        Demuxer::Ptr demuxer = this->shared_from_this();
        if (createTrackList() != -1)
        {
            m_ThreadState.store(STATE_WORKING, std::memory_order_seq_cst);
            m_Thread = new std::thread([demuxer]()
                                       { demuxer->demuxer_thread(); });
            // todo: 发送消息
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                auto track = m_TrackList[i].lock();
                if (track != nullptr)
                {
                    track->stop(); // 停止track
                }
            }
            return -1;
        }
    }
    return result;
}

int Demuxer::createTrackList()
{
    Demuxer::Ptr demuxer = this->shared_from_this();
    // 3.获取音视频流信息
    if (avformat_find_stream_info(m_AVFormatContext.get(), NULL) < 0)
    {
        qDebug("DecoderBase::InitFFDecoder avformat_find_stream_info fail.");
        return -1;
    }

    // 4.获取音视频流索引
    Track::Ptr track;
    for (uint32_t i = 0; i < m_AVFormatContext->nb_streams; i++)
    {
        switch (m_AVFormatContext->streams[i]->codecpar->codec_type)
        {
        case AVMEDIA_TYPE_AUDIO:
            /* code */
            track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_AUDIO, m_PacketQueue0);
            if (track->init() != -1)
            {
                track->start();
                m_TrackList[0] = track;
                m_TrackMap.emplace(i, track);
            }
            else
            {
                return -1;
            }
            break;
        case AVMEDIA_TYPE_VIDEO:
            /* code */
            track = std::make_shared<Track>(i, demuxer, AVMEDIA_TYPE_VIDEO, m_PacketQueue1);
            if (track->init() != -1)
            {
                track->start();
                m_TrackList[0] = track;
                m_TrackMap.emplace(i, track);
            }
            else
            {
                return -1;
            }
            break;

        default:
            break;
        }
    }
    return 0;
}

void Demuxer::notify()
{
    int pauseState = m_PauseState.exchange(STATE_WORKING, std::memory_order_seq_cst);
    if (pauseState == STATE_PAUSE)
    {
        m_PauseCond.notify_one();
    }
}

void Demuxer::sync()
{
    if (m_Thread != nullptr)
    {
        m_Thread->join();
    }
}

void Demuxer::stop()
{
    if (m_Thread != nullptr)
    {
        m_ThreadState.store(STATE_STOPPED, std::memory_order_seq_cst);
        notify();
    }
}

void Demuxer::demuxer_thread()
{
    while (true)
    {
        m_PauseState.store(STATE_PAUSE);
        if(m_ThreadState==STATE_STOPPED || m_ThreadState==STATE_STOPING){
            break;
        }

        if(m_ThreadState==STATE_PAUSING || m_PacketQueue0->isFull() || m_PacketQueue1->isFull()){//保证两个队列都至少有一个空位
            std::unique_lock<std::recursive_mutex> lock(m_PauseMutex);
            m_PauseCond.wait(lock);
            continue;
        }
        m_PauseState.store(STATE_WORKING);
        //解复用添加到队列中
        {
            AVPacket *packet=av_packet_alloc();
            std::unique_ptr<AVPacket> packet_ptr=std::make_unique<AVPacket>(packet,[](AVPacket *ptr){
                if(ptr!=nullptr)
                    av_packet_unref(ptr);
            });

            std::lock_guard<std::mutex> lock(m_RscMutex);
            int result = av_read_frame(m_AVFormatContext.get(), packet_ptr.get());
            if (result!=0)
            {
                if(result==AVERROR_EOF){
                    m_ThreadState.store(STATE_STOPING, std::memory_order_seq_cst);
                }else{
                    m_ThreadState.store(STATE_STOPPED, std::memory_order_seq_cst);
                }
            }else{
                auto iter=m_TrackMap.find(packet_ptr->stream_index);
                if(iter!=m_TrackMap.end()){
                    auto track=iter->second;
                    if(auto track_ptr=track.lock()){
                        track_ptr->append_packet(std::move(packet_ptr));
                    }
                }
            }
        }
    }
    //todo: 清理缓存

    //等待track线程结束
    for(int i=0;i<2;i++){
        auto track=m_TrackList[i].lock();
        if(track!=nullptr){
            track->notify();//通知获取解复用器的结果
        }
    }
    for(int i=0;i<2;i++){
        auto track=m_TrackList[i].lock();
        if(track!=nullptr){
            track->sync(); //等待track线程结束
        }
    }
}

void Demuxer::play(){

}
void Demuxer::pause(){

}
void Demuxer::seek(long position){

}


