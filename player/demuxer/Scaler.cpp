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
bool Scaler::stop_condition(){
    return m_frame_queue->is_empty();
}
