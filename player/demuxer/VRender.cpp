#include "VRender.h"
#include "VideoFrameScaler.h"

void VRender::append_frame(std::shared_ptr<AVFrame> frame){
    m_frame_queue->append_frame(std::move(frame));
    notify();
}
void VRender::set_video_frame_scaler(std::shared_ptr<VideoFrameScaler> frame_scaler){
   m_frame_scaler=frame_scaler;
   m_frame_queue=frame_scaler->get_scale_frame_queue();
   m_time_base=frame_scaler->get_time_base();
}

 bool VRender::pause_condition(){
    return m_frame_queue->is_empty();
 }
 bool VRender::stop_condition(){
    return m_frame_queue->is_empty();
 }
 int VRender::init(){
    m_frame_scaler->init_sws_context(m_screen_width,m_screen_height);
    return 0;
 }

 void VRender::seek(long position){
    clean_func();
 }

 void VRender::clean_func(){
    m_frame_queue->clear();
 }
