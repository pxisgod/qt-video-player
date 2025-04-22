#include "Sdl2AudioRender.h"
#include "AudioDecoder.h"
#include <mutex>
#include <qdebug.h>



void Sdl2AudioRender::Init()
{
    int result = -1;
    do
    {
        if (SDL_Init(SDL_INIT_AUDIO))
        { // 支持AUDIO
            //qDebug( "Could not initialize SDL - %s", SDL_GetError());
            break;
        }
        m_audioSpec.freq = AUDIO_DST_SAMPLE_RATE1;
        m_audioSpec.format = AUDIO_S16SYS;
        m_audioSpec.channels = AUDIO_DST_CHANNEL_COUNTS1;
        m_audioSpec.silence = 0;
        m_audioSpec.samples = ACC_NB_SAMPLES1;
        m_audioSpec.callback = nullptr; // 因为是推模式，所以这里为 nullptr
                                        // 打开音频设备
        if ((m_deviceId = SDL_OpenAudioDevice(nullptr, 0, &m_audioSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE)) < 2)
        {
            break;
        }

        // play audio
        SDL_PauseAudioDevice(m_deviceId, 0);

        m_thread = new std::thread(CreateSdl2WaitingThread, this);
        result = 0;
    } while (false);

    if (result != 0)
    {
        UnInit();
    }
}
void Sdl2AudioRender::ClearAudioCache()
{
    /**
    std::unique_lock<std::mutex> lock(m_Mutex);
    while (!m_AudioFrameQueue.empty())
    {
        AudioFrame *audioFrame = (m_AudioFrameQueue.front());
        delete audioFrame;
        m_AudioFrameQueue.pop();
    }
    SDL_ClearQueuedAudio(m_deviceId);
    lock.unlock();
     */
}

void Sdl2AudioRender::RenderAudioFrame(uint8_t *pData, int dataSize)
{
    if (pData != nullptr && dataSize > 0)
    {
        // temp resolution, when queue size is too big.
        while (m_AudioFrameQueue.size() >= MAX_QUEUE_BUFFER_SIZE && !m_Exit)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }

        std::unique_lock<std::mutex> lock(m_Mutex);
        AudioFrame *audioFrame = new AudioFrame(pData, dataSize);
        m_AudioFrameQueue.push(audioFrame);
        m_Cond.notify_all();
        lock.unlock();
    }
}
void Sdl2AudioRender::UnInit()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Exit = true;
    m_Cond.notify_all();
    lock.unlock();

    if (m_thread != nullptr)
    {
        m_thread->join();
        m_thread = nullptr;
    }
    ClearAudioCache();
    if (m_deviceId != -1)
    {
        SDL_CloseAudioDevice(m_deviceId);
    }
}

void Sdl2AudioRender::StartRender()
{
    while (!m_Exit)
    {
        if (m_AudioFrameQueue.empty())
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            if(m_AudioFrameQueue.empty()){
                m_Cond.wait(lock);
            }
            lock.unlock();
        }
        else
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            AudioFrame *audioFrame = m_AudioFrameQueue.front();
            if (nullptr != audioFrame)
            {
                m_AudioFrameQueue.pop();
                SDL_QueueAudio(m_deviceId, audioFrame->data, audioFrame->dataSize);
                SDL_Delay(1);
                delete audioFrame;
            }
            lock.unlock();
        }
    }
}

void Sdl2AudioRender::CreateSdl2WaitingThread(Sdl2AudioRender *sdl2AudioRender)
{
    sdl2AudioRender->StartRender();
}