#include "Render.h"
#include "Render.h"

void Render::start()
{
    Render::Ptr track = this->shared_from_this();
    m_Thread = new std::thread([track]()
                               {track->render_thread(); });
}

void Render::append_frame(std::unique_ptr<AVFrame> frame){
    m_FrameQueue->append_frame(std::move(frame));
    notify();
}

void Render::notify()
{
    int pauseState = m_PauseState.exchange(STATE_WORKING, std::memory_order_seq_cst);
    if (pauseState == STATE_PAUSE)
    {
        m_PauseCond.notify_one();
    }
}

void Render::sync()
{
    if (m_Thread != nullptr)
    {
        m_Thread->join();
    }
}

void Render::stop()
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

void Render::render_thread()
{
   
    
}



void Render::play(){
    int expected=STATE_PAUSING;
    if(m_ThreadState.compare_exchange_strong(expected,STATE_WORKING,std::memory_order_seq_cst)){
        this->notify();
        this->play_0();
    }
}

void Render::pause(){
    int expected=STATE_WORKING;
    if(m_ThreadState.compare_exchange_strong(expected,STATE_PAUSING,std::memory_order_seq_cst)){
        this->notify();
        this->play_0();
    }
}

void Render::seek(long position){

}
