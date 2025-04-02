#ifndef COMMON_H
#define COMMON_H
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/frame.h>
    #include <libavutil/time.h>
    #include <libavcodec/jni.h>
    #include <libavutil/samplefmt.h>
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
    #include <libavutil/audio_fifo.h> 
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
};
#include <qdebug.h>
#include <mutex>
#include <condition_variable>

enum PauseState {
    STATE_PLAYING,
    STATE_PAUSE
};

enum ThreadState {
    STATE_WORKING,
    STATE_PAUSING,
    STATE_STOPING,
    STATE_STOPPED
};

enum PlayerCBType {
    MSG_INIT_ERROR, //解码器初始化失败
    MSG_DEMUXER_START,//解码器初始化成功 
    MSG_DEMUXER_STOP, //关闭解码器 
    MSG_RENDER_TIME, //完成渲染时间
    MSG_PLAY_FINISH,  //播放完成
};

#define MAX_PACKET_QUEUE_SIZE 64
#define MAX_FRAME_QUEUE_SIZE 64
#define MAX_PACKET_RESERVE_SIZE 32
#define MAX_FRAME_RESERVE_SIZE 32

typedef struct PakcetQueue{
    std::unique_ptr<AVPacket> packet_queue[MAX_PACKET_QUEUE_SIZE];
    uint16_t r_index=0;
    uint16_t w_index=0;
    //std::recursive_mutex mutex;
    //std::condition_variable cond;
    int reserve_size=MAX_PACKET_RESERVE_SIZE; 

    bool isEmpty(){
        return r_index==w_index;
    };
    bool isFull(){
        return (w_index<r_index && w_index+reserve_size+1>=r_index) ||
                 (w_index>r_index && (w_index+reserve_size+1)%MAX_FRAME_QUEUE_SIZE>=r_index);
    };
    void append_packet(std::unique_ptr<AVPacket> packet){
        packet_queue[w_index]=std::move(packet);
        w_index=(w_index+1)%MAX_PACKET_QUEUE_SIZE;
    };
    AVPacket* remove_packet(){
        int rIndex=r_index;
        r_index=(r_index+1)%MAX_PACKET_QUEUE_SIZE;
        AVPacket * packet= packet_queue[rIndex].release();
        packet_queue[rIndex]=nullptr;
        return packet;
    };
    void clear(){
        for(int i=0;i<MAX_PACKET_QUEUE_SIZE;i++){
            packet_queue[i]=nullptr;
        }
        r_index=0;
        w_index=0;
    };
}PacketQueue;

typedef struct FrameQueue{
    std::unique_ptr<AVFrame> frame_queue[MAX_FRAME_QUEUE_SIZE];
    uint16_t r_index=0;
    uint16_t w_index=0;
    //std::recursive_mutex mutex;
    //std::condition_variable cond;
    int reserve_size=MAX_FRAME_RESERVE_SIZE;//预留空间,因为一次解码出来的frame可能不只1

    
    bool isEmpty(){
        return r_index==w_index;
    };
    bool isFull(){
        return (w_index<r_index && w_index+reserve_size+1>=r_index) ||
                 (w_index>r_index && (w_index+reserve_size+1)%MAX_FRAME_QUEUE_SIZE>=r_index);
    };
    void append_frame(std::unique_ptr<AVFrame> packet){
        frame_queue[w_index]=std::move(packet);
        w_index=(w_index+1)%MAX_FRAME_QUEUE_SIZE;
    };
    AVFrame* remove_frame(){
        int rIndex=r_index;
        r_index=(r_index+1)%MAX_FRAME_QUEUE_SIZE;
        AVFrame* frame= frame_queue[rIndex].release();
        frame_queue[rIndex]=nullptr;
        return frame;
    };
    void clear(){
        for(int i=0;i<MAX_PACKET_QUEUE_SIZE;i++){
            frame_queue[i]=nullptr;
        }
        r_index=0;
        w_index=0;
    };
}FrameQueue;

//解复用消息类型
typedef struct DemuxerMsg{
    int msgType;
    long time;
}DemuxerMsg;

#endif