#ifndef THREAD_CHAIN_H
#define THREAD_CHAIN_H
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>
#include <atomic>
#include <queue>
#include "EventListener.h"
#include "Clock.h"

enum ThreadState
{
    THREAD_RUNNING,
    THREAD_PAUSE,
    THREAD_WAITING
};

enum WorkState
{
    IS_WORKING,
    IS_PAUSED,
    IS_STOPING,
    IS_STOPPED
};

class ThreadChain : public std::enable_shared_from_this<ThreadChain>,
                    public EventListener<ThreadMsg>,
                    public EventNotifier<ThreadMsg>
{
public:
    using S_Ptr = std::shared_ptr<ThreadChain>;
    using W_Ptr = std::weak_ptr<ThreadChain>;

    explicit ThreadChain()
    {
        m_work_state.store(IS_STOPPED, std::memory_order_seq_cst);//默认工作状态为STOPPED
        m_level = 0;
    }
    virtual ~ThreadChain()
    {
        sync();
    }
    bool operator<(const ThreadChain &b)
    {
        return m_level < b.m_level;
    }
    int get_thread_state()
    {
        return m_thread_state.load(std::memory_order_seq_cst);
    }
    int get_work_state()
    {
        return m_work_state.load(std::memory_order_seq_cst);
    }


public:
    void add_thread(ThreadChain::S_Ptr child); 
    void notify();
    void sync();

    void try_stop_0();
    void start();
    void thread_func(); 
    int init(long system_time);
    void seek(long pts_time,long system_time);
    void play(long system_time);
    void pause();
    void try_stop();
    void stop();
    void uninit();

    virtual int stop_0();
    virtual bool pause_condition(int work_state) { return false; }
    virtual bool stop_condition() { return false; }
    virtual long get_wait_time(){return 0;}
    virtual void deal_neg_wait_time(){}
    virtual int do_init(long system_time){return 0;}
    virtual void do_seek(long pts_time,long system_time){}
    virtual void do_play(long system_time){}
    virtual void do_pause(){}
    virtual void do_try_stop(){}
    virtual void do_stop(){}
    virtual void do_uninit(){}
    virtual int thread_init(){return 0;} //线程初始化
    virtual int work_func() { return 0; } //-1异常，0正常处理，1处理结束
    virtual void clean_func() {} //清理资源
 
    static std::list<ThreadChain::S_Ptr> get_all_thread(ThreadChain::S_Ptr thread);
    static ThreadChain::S_Ptr get_root(ThreadChain::S_Ptr thread);
    static bool is_root(ThreadChain::S_Ptr thread);
public:
    std::mutex m_rsc_mutex;                    // 资源锁
private:
    int m_level; // 在树中的层级
    std::list<ThreadChain::W_Ptr> m_child_list;
    ThreadChain::S_Ptr m_father;
    std::thread *m_thread;
    std::atomic<int> m_thread_state;           // 线程状态
    std::atomic<int> m_work_state;             // 工作状态
    std::recursive_mutex m_thread_mutex;       // 暂停条件锁
    std::condition_variable_any m_thread_cond; // 暂停条件
    static thread_local std::list<ThreadChain::S_Ptr> m_all_thread; //所有线程的强引用
};

#endif