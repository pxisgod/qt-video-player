#ifndef V_RENDER_H
#define V_RENDER_H
#include "ThreadChain.h"

class VideoFrameScaler;
class VRender:public ThreadChain{
private:
    std::shared_ptr<VideoFrameScaler> m_frame_scaler;
};
#endif;
