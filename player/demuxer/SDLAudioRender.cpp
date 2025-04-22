#include "SDLAudioRender.h"
#include "AudioFrameScaler.h"

int SDLAudioRender::thread_init()
{
    if (SDL_Init(SDL_INIT_AUDIO))
    {
        qDebug("SDL2初始化失败 - %s", SDL_GetError());
        return -1;
    }
    else
    {
        m_audio_spec.freq = AUDIO_DST_SAMPLE_RATE;
        m_audio_spec.format = AUDIO_S16SYS;
        m_audio_spec.channels = AUDIO_DST_CHANNEL_COUNTS;
        m_audio_spec.silence = 0;
        m_audio_spec.samples = AUDIO_NB_SAMPLES;
        m_audio_spec.callback = audio_callback;
        m_audio_spec.userdata = this;
        if ((m_device_id = SDL_OpenAudioDevice(nullptr, 0, &m_audio_spec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE)) < 2)
        {
            return -1;
        }
        SDL_PauseAudioDevice(m_device_id, 0);
        return 0;
    }
}

int SDLAudioRender::work_func()
{
    decltype(m_command_list) command_list;
    {
        std::lock_guard<std::mutex> lock(m_command_mutex);
        m_command_list.swap(command_list);
    }
    for (auto &command : command_list)
    {
        switch (command)
        {
        case CMD_PLAY:
            SDL_PauseAudioDevice(m_device_id, 0);//播放
            break;
        case CMD_PAUSE:
            SDL_PauseAudioDevice(m_device_id, 1);//暂停
            break;
        default:
            break;
        }
    }
    return 0;
}

void SDLAudioRender::clean_func()
{
    if (m_device_id != -1)
    {
        SDL_Delay(100);//等待音频播放完成
        SDL_CloseAudioDevice(m_device_id);
        m_device_id = -1;
    }
}

void SDLAudioRender::do_play(long system_time)
{
    ARender::do_play(system_time);
    std::lock_guard<std::mutex> lock(m_command_mutex);
    m_command_list.push_back(CMD_PLAY);
    notify();
}

void SDLAudioRender::do_pause()
{
    std::lock_guard<std::mutex> lock(m_command_mutex);
    m_command_list.push_back(CMD_PAUSE);
    notify();
}

bool SDLAudioRender::pause_condition(int work_state)
{
    return m_command_list.empty();
}

void SDLAudioRender::audio_callback(void *userdata, Uint8 *stream, int len)
{
    SDLAudioRender *audio_render = static_cast<SDLAudioRender *>(userdata);
    std::shared_ptr<AudioSampleQueue> sample_queue = audio_render->m_sample_queue;
    std::lock_guard<std::mutex> lock(audio_render->m_rsc_mutex);
    long pts_time=sample_queue->calculate_pts_time();
    long delay = audio_render->get_clock()->get_target_delay(pts_time,get_system_current_time());  
    qDebug("音频延迟时间：%ld",delay);
    long skip_per_samples=100;
    long delay_samples=delay * AUDIO_DST_SAMPLE_RATE / 1000; //延迟样本数
    long reserve_samples=sample_queue->get_length();//剩余样本数
    long need_samples=len/(AUDIO_DST_CHANNEL_COUNTS)/(AUDIO_SAMPLE_SIZE);//需要样本数
    if(delay<-100){
        //延迟过大，跳过样本
        long skip_length=std::min(-delay_samples,reserve_samples);
        reserve_samples-=skip_length;
        delay_samples+=skip_length;
        sample_queue->skip(skip_length);//需要样本数不变
    }else if(delay>100){
        //延迟过小，重复样本
        long rollback_length=std::min(delay_samples,need_samples);
        need_samples-=rollback_length;
        delay_samples-=rollback_length;
        while(rollback_length>0){
            long x=std::min(reserve_samples,rollback_length);
            for(long i=0;i<x;i++){
                for(int j=0;j<(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE);j++){
                    stream[i*(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE)+j]=0;
                }
            }
            //sample_queue->rollback(x);
            rollback_length-=x;
        }
    }
    for(int i=0;i<need_samples;i++)
    {
        if(delay_samples<0 && skip_per_samples<=0 && reserve_samples>0){
            //跳过样本
            for(int j=0;j<(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE);j++){
                sample_queue->read_sample();
            }
            i--;
            reserve_samples--;
            skip_per_samples=100;
            delay_samples++;
        }else if(delay_samples>0 && skip_per_samples<=0 && reserve_samples>0){
            //重复样本
            for(int j=0;j<(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE);j++){
                stream[i*(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE)+j]=sample_queue->read_sample();
            }
            sample_queue->rollback();
            skip_per_samples=100;
            delay_samples--;
        }else{
            skip_per_samples--;
            if(reserve_samples<=0){
                //没有样本
                for(int j=0;j<(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE);j++){
                    stream[i*(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE)+j]=0;
                }
            }else{
                //读取样本
                for(int j=0;j<(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE);j++){
                    stream[i*(AUDIO_DST_CHANNEL_COUNTS)*(AUDIO_SAMPLE_SIZE)+j]=sample_queue->read_sample();
                }
            }
            reserve_samples--;
        }
    }
    //设置时钟
    audio_render->get_clock()->set_clock(pts_time,get_system_current_time(),false);
    //通知render可能结束
    audio_render->notify();
}