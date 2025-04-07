#include "Scaler.h"
#include "Track.h"
Scaler::Scaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track){
    m_frame_queue=frame_queue;
    m_track=track;
    m_clock=track->get_clock();
}
void Scaler::append_frame(std::shared_ptr<AVFrame> frame){
    m_frame_queue->append_frame(frame);
    notify();
}
void Scaler::render_finish(){
    m_frame_queue->remove_frame_1();
    m_track->notify();
}
bool Scaler::pause_condition(){
    return m_frame_queue->is_empty() ||m_scale_frame_queue->is_full();
}
bool Scaler::stop_condition(){
    return m_frame_queue->is_empty();
}
long Scaler::get_wait_time(){
    return 0;
}
void Scaler::deal_neg_wait_time(){
}
int Scaler::init(){
    m_scale_frame_queue = std::make_shared<FrameQueue>();//创建scale后的frame_queue
    ThreadChain::init(); //设置消息链
    return 0;
}
void Scaler::seek(long position){
    clean_func();
}
void Scaler::clean_func(){
    m_scale_frame_queue->clear();
}