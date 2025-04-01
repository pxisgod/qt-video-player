#ifndef SDL2_AUDIORENDER_H
#define SDL2_AUDIORENDER_H
#define MAX_QUEUE_BUFFER_SIZE 3

#include "AudioRender.h"
#include <cstdint>
#include <queue>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <SDL2/SDL.h>


class Sdl2AudioRender : public AudioRender {
public:
    Sdl2AudioRender(){}
    virtual ~Sdl2AudioRender(){}
    virtual void Init();
    virtual void ClearAudioCache();
    virtual void RenderAudioFrame(uint8_t *pData, int dataSize);
    virtual void UnInit();
private:
    void StartRender();
    static void CreateSdl2WaitingThread(Sdl2AudioRender *sdl2AudioRender);
    std::queue<AudioFrame *> m_AudioFrameQueue;
    std::thread *m_thread = nullptr;
    std::mutex   m_Mutex;
    std::condition_variable m_Cond;
    volatile bool m_Exit = false;
    SDL_AudioSpec m_audioSpec;
    SDL_AudioDeviceID m_deviceId=-1;

    
};
#endif
