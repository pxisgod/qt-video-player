#include "ARender.h"
#include "AudioFrameScaler.h"

void ARender::append_samples(std::shared_ptr<uint8_t> samples,int size,long pts){
    m_sample_queue->append_samples(samples,size,pts);
    notify();
}
void ARender::set_audio_frame_scaler(std::shared_ptr<AudioFrameScaler> frame_scaler){
   m_frame_scaler=frame_scaler;
   m_sample_queue=frame_scaler->get_scale_sample_queue();
   m_clock=frame_scaler->get_clock();
}

 bool ARender::pause_condition(int work_state){
    return m_sample_queue->is_empty();
 }
 bool ARender::stop_condition(){
    return m_sample_queue->is_empty();
 }

void ARender::do_seek(long pts_time,long system_time){
   m_clock->set_clock(pts_time,system_time,true);
}

void ARender::do_play(long system_time){
   m_clock->resync_clock(system_time);
 }