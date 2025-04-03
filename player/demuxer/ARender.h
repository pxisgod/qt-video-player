#ifndef A_RENDER_H
#define A_RENDER_H
#include "ThreadChain.h"

class AudioFrameScaler;
class ARender:public ThreadChain{
private:
    std::shared_ptr<AudioFrameScaler> m_frame_scaler;
};
#endif