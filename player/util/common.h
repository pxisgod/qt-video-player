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
    std::unique_ptr<AVPacket> packet_queue[MAX_PACKET_QUEUE_SIZE];
    uint16_t r_index = 0; // 读指针
    uint16_t w_index = 0; // 写指针
    uint16_t b_index = 0; // 基指针
    // std::recursive_mutex mutex;
    // std::condition_variable cond;
    int reserve_size = MAX_PACKET_RESERVE_SIZE;

    // 已经没有可以读的空间
    bool is_empty()
    {
        return r_index == w_index;
    };
    // 已经没有可以写的空间
    bool is_full()
    {
        return (w_index < b_index && w_index + reserve_size + 1 >= b_index) ||
               (w_index > b_index && (w_index + reserve_size + 1) % MAX_PACKET_QUEUE_SIZE >= b_index);
    };
    void append_packet(std::unique_ptr<AVPacket> packet)
    {
        packet_queue[w_index] = std::move(packet);
        w_index = (w_index + 1) % MAX_PACKET_QUEUE_SIZE;
    };
    AVPacket *read_packet_1()
    { // 不更新b_index
        int index = r_index;
        r_index = (r_index + 1) % MAX_PACKET_QUEUE_SIZE;
        AVPacket *packet = packet_queue[index].get();
        return packet;
    };
    AVPacket *read_packet_2()
    { // 同时更新b_index
        int index = r_index;
        r_index = (r_index + 1) % MAX_PACKET_QUEUE_SIZE;
        b_index = r_index;
        AVPacket *packet = packet_queue[index].release();
        packet_queue[index] = nullptr;
        return packet;
    };
    void remove_packet()
    {
        int index = b_index;
        b_index = (b_index + 1) % MAX_PACKET_QUEUE_SIZE;
        packet_queue[index] = nullptr;
    };
    void rollback()
    { // 读指针回滚到基指针
        r_index = b_index;
    }
    void clear()
    {
        for (int i = 0; i < MAX_PACKET_QUEUE_SIZE; i++)
        {
            packet_queue[i] = nullptr;
        }
        r_index = 0;
        w_index = 0;
        b_index = 0;
    };
} PacketQueue;

typedef struct FrameQueue
{
    std::unique_ptr<AVFrame, void (*)(AVFrame *)> frame_queue[MAX_FRAME_QUEUE_SIZE] = {
        std::unique_ptr<AVFrame, void (*)(AVFrame *)>(nullptr, [](AVFrame *frame) { if (frame) av_frame_free(&frame); })};
    uint16_t r_index = 0;
    uint16_t w_index = 0;
    uint16_t b_index = 0;
    // std::recursive_mutex mutex;
    // std::condition_variable cond;
    int reserve_size = MAX_FRAME_RESERVE_SIZE; // 预留空间,因为一次解码出来的frame可能不只1

    // 已经没有可以读的空间
    bool is_empty()
    {
        return r_index == w_index;
    };
    // 已经没有可以写的空间
    bool is_full()
    {
        return (w_index < b_index && w_index + reserve_size + 1 >= b_index) ||
               (w_index > b_index && (w_index + reserve_size + 1) % MAX_FRAME_QUEUE_SIZE >= b_index);
    };
    void append_frame(std::unique_ptr<AVFrame, void (*)(AVFrame *)> frame)
    {
        frame_queue[w_index] = std::move(frame);
        w_index = (w_index + 1) % MAX_FRAME_QUEUE_SIZE;
    };
    AVFrame *read_frame_1()
    { // 不更新b_index
        int index = r_index;
        r_index = (r_index + 1) % MAX_FRAME_QUEUE_SIZE;
        AVFrame *frame = frame_queue[index].get();
        return frame;
    };
    AVFrame *read_frame_2()
    { // 同时更新b_index
        int index = r_index;
        r_index = (r_index + 1) % MAX_FRAME_QUEUE_SIZE;
        b_index = r_index;
        AVFrame *frame = frame_queue[index].release();
        frame_queue[index] = nullptr;
        return frame;
    };
    void remove_frame()
    {
        int index = b_index;
        b_index = (b_index + 1) % MAX_FRAME_QUEUE_SIZE;
        frame_queue[index] = nullptr;
    };
    void rollback()
    { // 读指针回滚到基指针
        r_index = b_index;
    }
    void clear()
    {
        for (int i = 0; i < MAX_FRAME_QUEUE_SIZE; i++)
        {
            frame_queue[i] = nullptr;
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