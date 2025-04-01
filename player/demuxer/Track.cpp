#include "Track.h"
#include "Demuxer.h"

int Track::init()
{
    // 5.获取解码器参数
    AVCodecParameters *codecParameters = m_Demuxer->getAVFormatContext()->streams[m_StreamId]->codecpar;

    // 6.获取解码器
    AVCodec *avCodec = const_cast<AVCodec *>(avcodec_find_decoder(codecParameters->codec_id));
    if (avCodec == nullptr)
    {
        qDebug("DecoderBase::InitFFDecoder avcodec_find_decoder fail.");
        return -1;
    }

    // 7.创建解码器上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    m_AVCodecContext = std::make_shared<AVCodecContext>(avCodecContext, [](AVCodecContext *ptr)
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
    return 0;
}

void Track::start()
{
    Track::Ptr track = this->shared_from_this();
    m_Thread = new std::thread([track]()
                               {track->decoder_thread(); });
}

void Track::append_packet(std::unique_ptr<AVPacket> packet){
    m_PacketQueue->append_packet(std::move(packet));
    notify();
    
}

void Track::notify()
{
    int pauseState = m_PauseState.exchange(STATE_WORKING, std::memory_order_seq_cst);
    if (pauseState == STATE_PAUSE)
    {
        m_PauseCond.notify_one();
    }
}

void Track::sync()
{
    if (m_Thread != nullptr)
    {
        m_Thread->join();
    }
}

void Track::stop()
{
    if (m_Thread != nullptr)
    {
        m_ThreadState.store(STATE_STOPPED, std::memory_order_seq_cst);
        int pauseState = m_PauseState.exchange(STATE_WORKING, std::memory_order_seq_cst);
        if (pauseState == STATE_PAUSE)
        {
            m_PauseCond.notify_one();
        }
        m_Thread->join();
    }
}

void Track::decoder_thread()
{
    while (true)
    {
        m_PauseState.store(STATE_PAUSE);
        if(m_ThreadState==STATE_STOPPED || m_Demuxer->get_thread_state()==STATE_STOPPED){
            m_ThreadState=STATE_STOPPED;
            break;
        }

        if(m_ThreadState==STATE_PAUSING || m_PacketQueue->isEmpty()){
            if(m_ThreadState==STATE_PAUSING){
                std::unique_lock<std::recursive_mutex> lock(m_PauseMutex);
                m_PauseCond.wait(lock);
                continue;
            }else if(m_ThreadState==STATE_WORKING){
                if(m_Demuxer->get_thread_state()==STATE_STOPING){
                    int expected=STATE_WORKING;
                    if(m_ThreadState.compare_exchange_strong(expected,STATE_STOPING, std::memory_order_seq_cst)){
                        break;
                    }
                }else{
                    std::unique_lock<std::recursive_mutex> lock(m_PauseMutex);
                    m_PauseCond.wait(lock);
                    continue;
                }
            }
            
        }
        m_PauseState.store(STATE_WORKING);
        //解码
        {
            std::lock_guard<std::mutex> lock(m_RscMutex); //资源锁
            int rIndex=m_PacketQueue->r_index;
            if(avcodec_send_packet(m_AVCodecContext.get(), m_PacketQueue->remove_packet()) ==0) {
                int frameCount = 0;
                AVFrame *frame=av_frame_alloc();
                std::unique_ptr<AVFrame> frame_ptr=std::make_unique<AVFrame>(frame,[](AVFrame *ptr){
                    if(ptr!=nullptr)
                        av_frame_unref(ptr);
                });
                while (avcodec_receive_frame(m_AVCodecContext.get(), frame_ptr.get()) == 0) {
                    m_FrameQueue->append_frame(std::move(frame_ptr));
                    frame_ptr=std::make_unique<AVFrame>(frame,[](AVFrame *ptr){
                        if(ptr!=nullptr)
                            av_frame_unref(ptr);
                     });
                }
            }else{
                //解码失败
                m_ThreadState=STATE_STOPPED;
            }
        }
    }
    //todo: 清理缓存
    avcodec_flush_buffers(m_AVCodecContext.get());

    //等待render线程结束
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
    //通知demuxer,解码结束
    if(m_ThreadState==STATE_STOPING){
        m_ThreadState=STATE_STOPPED;//由demuxera发起的正常停止请求
    }else{
        m_Demuxer->stop(); //关闭Demuxer
    }
    
}

void Track::decode()
{
}



void Track::play(){
    int expected=STATE_PAUSING;
    if(m_ThreadState.compare_exchange_strong(expected,STATE_WORKING,std::memory_order_seq_cst)){
        this->notify();
        this->play_0();
    }
}

void Track::pause(){
    int expected=STATE_WORKING;
    if(m_ThreadState.compare_exchange_strong(expected,STATE_PAUSING,std::memory_order_seq_cst)){
        this->notify();
        this->play_0();
    }
}

void Track::seek(long position){

}
