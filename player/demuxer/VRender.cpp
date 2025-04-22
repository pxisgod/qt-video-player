#include "VRender.h"
#include "VideoFrameScaler.h"

void VRender::append_frame(std::shared_ptr<AVFrame> frame){
    m_frame_queue->append_frame(frame);
    notify();
}
void VRender::set_video_frame_scaler(std::shared_ptr<VideoFrameScaler> frame_scaler){
   m_frame_scaler=frame_scaler;
   m_frame_queue=frame_scaler->get_scale_frame_queue();
   m_clock=frame_scaler->get_clock();
}

 bool VRender::pause_condition(int work_state){
    return m_frame_queue->is_empty();
 }
 bool VRender::stop_condition(){
    return m_frame_queue->is_empty();
 }

 int VRender::do_init(long system_time){
    m_frame_scaler->init_sws_context(m_screen_width,m_screen_height);
    return 0;
 }

 void VRender::do_seek(long pts_time,long system_time){
   m_clock->set_clock(pts_time,system_time,true);
 }

 void VRender::do_play(long system_time){
   m_clock->resync_clock(system_time);
 }
