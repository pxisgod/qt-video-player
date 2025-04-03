#include "VRender.h"
#include "VideoFrameScaler.h"

void VRender::append_frame(std::unique_ptr<AVFrame> frame){
    m_frame_queue->append_frame(std::move(frame));
    notify();
}

 bool VRender::pause_condition(){
    return m_frame_queue->is_empty();
 }
 bool VRender::stop_condition(){
    return m_frame_queue->is_empty();
 }
 int VRender::init(){
    m_frame_scaler->init_sws_context(screen_width,screen_height);
    return 0;
 }

 void VRender::seek(long position){
    clean_func();
 }

 void VRender::clean_func(){
    m_frame_queue->clear();
 }
