#ifndef SDL_AUDIO_RENDER_H
#define SDL_AUDIO_RENDER_H
#include "ARender.h"
#include <SDL2/SDL.h>
#include "SDLRender.h"
enum Command
{
    CMD_PLAY,
    CMD_PAUSE
};
class SDLAudioRender : public ARender,public SDL_Render
{
public:
    virtual int thread_init();
    virtual int work_func();
    virtual void clean_func();
    virtual void do_play(long system_time);
    virtual void do_pause();
    virtual bool pause_condition(int work_state);
    static void audio_callback (void *userdata, Uint8 * stream, int len);

private:
    SDL_AudioSpec m_audio_spec;
    SDL_AudioDeviceID m_device_id=-1;
    std::list<Command> m_command_list;
    std::mutex m_command_mutex;
};
#endif