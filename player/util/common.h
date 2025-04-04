#ifndef COMMON_H
#define COMMON_H
extern "C"
{
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
#include <memory>

enum PlayerCBType
{
    MSG_INIT_ERROR,    // 解码器初始化失败
    MSG_DEMUXER_START, // 解码器初始化成功
    MSG_DEMUXER_STOP,  // 关闭解码器
    MSG_RENDER_TIME,   // 完成渲染时间
    MSG_PLAY_FINISH,   // 播放完成
};

#define MAX_PACKET_QUEUE_SIZE 64
#define MAX_FRAME_QUEUE_SIZE 64
#define MAX_PACKET_RESERVE_SIZE 32
#define MAX_FRAME_RESERVE_SIZE 32

typedef struct PakcetQueue
{
    std::shared_ptr<AVPacket> packet_queue[MAX_PACKET_QUEUE_SIZE];
    uint16_t r_index = 0; // 读指针
    uint16_t w_index = 0; // 写指针
    uint16_t b_index = 0; // 基指针
    std::recursive_mutex mutex;
    std::condition_variable cond;
    int reserve_size = MAX_PACKET_RESERVE_SIZE;

    // 已经没有可以读的空间
    bool is_empty()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        return r_index == w_index;
    };
    // 已经没有可以写的空间
    bool is_full()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        return (w_index < b_index && w_index + reserve_size + 1 >= b_index) ||
               (w_index > b_index && (w_index + reserve_size + 1) % MAX_PACKET_QUEUE_SIZE >= b_index);
    };
    void append_packet(std::shared_ptr<AVPacket> packet)
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        packet_queue[w_index] = packet;
        w_index = (w_index + 1) % MAX_PACKET_QUEUE_SIZE;
    };
    std::shared_ptr<AVPacket> read_packet()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        return packet_queue[r_index];
    };
    void remove_packet_1()//b_index+1
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        int index = b_index;
        b_index = (b_index + 1) % MAX_PACKET_QUEUE_SIZE;
        packet_queue[index].reset();
    };
    void remove_packet_2()//r_index+1
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        int index = r_index;
        r_index = (r_index + 1) % MAX_PACKET_QUEUE_SIZE;
        packet_queue[index].reset();
    };
    void remove_packet_3()//r_index+1 && b_index=r_index
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        r_index = (r_index + 1) % MAX_PACKET_QUEUE_SIZE;
        for(;b_index!=r_index;){
            packet_queue[b_index].reset();
            b_index = (b_index + 1) % MAX_PACKET_QUEUE_SIZE;
        }
    };
    void rollback()
    { // 读指针回滚到基指针
        std::unique_lock<std::recursive_mutex> lock(mutex);
        r_index = b_index;
    }
    void clear()
    {
        std::unique_lock<std::recursive_mutex> lock(mutex);
        for (int i = 0; i < MAX_PACKET_QUEUE_SIZE; i++)
        {
            packet_queue[i].reset();
        }
        r_index = 0;
        w_index = 0;
        b_index = 0;
    };
} PacketQueue;

typedef struct FrameQueue
{
    std::shared_ptr<AVFrame> frame_queue[MAX_FRAME_QUEUE_SIZE];
    uint16_t r_index = 0;
    uint16_t w_index = 0;
    uint16_t b_index = 0;
    std::recursive_mutex mutex;
    std::condition_variable cond;
    int reserve_size = MAX_FRAME_RESERVE_SIZE; // 预留空间,因为一次解码出来的frame可能不只1

   // 已经没有可以读的空间
   bool is_empty()
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       return r_index == w_index;
   };
   // 已经没有可以写的空间
   bool is_full()
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       return (w_index < b_index && w_index + reserve_size + 1 >= b_index) ||
              (w_index > b_index && (w_index + reserve_size + 1) % MAX_PACKET_QUEUE_SIZE >= b_index);
   };
   void append_frame(std::shared_ptr<AVFrame> frame)
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       frame_queue[w_index] = frame;
       w_index = (w_index + 1) % MAX_PACKET_QUEUE_SIZE;
   };
   std::shared_ptr<AVFrame> read_frame()
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       return frame_queue[r_index];
   };
   void remove_frame_1()//b_index+1
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       int index = b_index;
       b_index = (b_index + 1) % MAX_PACKET_QUEUE_SIZE;
       frame_queue[index].reset();
   };
   void remove_frame_2()//r_index+1
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       int index = r_index;
       r_index = (r_index + 1) % MAX_PACKET_QUEUE_SIZE;
       frame_queue[index].reset();
   };
   void remove_frame_3()//r_index+1 && b_index=r_index
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       r_index = (r_index + 1) % MAX_PACKET_QUEUE_SIZE;
       for(;b_index!=r_index;){
            frame_queue[b_index].reset();
            b_index = (b_index + 1) % MAX_PACKET_QUEUE_SIZE;
       }
   };
   void rollback()
   { // 读指针回滚到基指针
       std::unique_lock<std::recursive_mutex> lock(mutex);
       r_index = b_index;
   }
   void clear()
   {
       std::unique_lock<std::recursive_mutex> lock(mutex);
       for (int i = 0; i < MAX_PACKET_QUEUE_SIZE; i++)
       {
        frame_queue[i].reset();
       }
       r_index = 0;
       w_index = 0;
       b_index = 0;
   };
} FrameQueue;

// 解复用消息类型
typedef struct DemuxerMsg
{
    int msgType;
    long time;
} DemuxerMsg;

#endif