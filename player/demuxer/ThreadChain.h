#ifndef THREAD_CHAIN_H
#define THREAD_CHAIN_H
#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>
#include <atomic>
#include <queue>

enum ThreadState
{
    THREAD_RUNNING,
    THREAD_PAUSE
};

enum WorkState
{
    IS_WORKING,
    IS_PAUSED,
    IS_STOPING,
    IS_STOPPED
};

enum PlayState
{
    PLAYING,
    PAUSE,
    STOPPED
};

class ThreadChain : public std::enable_shared_from_this<ThreadChain>
{
public:
    using S_Ptr = std::shared_ptr<ThreadChain>;
    using W_Ptr = std::weak_ptr<ThreadChain>;

    explicit ThreadChain()
    {
        m_work_state.store(IS_STOPPED, std::memory_order_seq_cst);//默认工作状态为STOPPED
        m_play_state = STOPPED; // 初始播放状态为STOPPED
        m_level = 0;
    }

    virtual ~ThreadChain()
    {
        if (m_thread != nullptr)
        {
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
    }

    bool operator<(const ThreadChain &b)
    {
        return m_level < b.m_level;
    }
    static std::list<ThreadChain::S_Ptr> get_all_thread(ThreadChain::S_Ptr thread)
    {
        std::list<ThreadChain::S_Ptr> leaf_list;
        leaf_list.push_back(thread);
        if (!thread->m_child_list.empty())
        {
            for (auto thread_w_ptr : thread->m_child_list)
            {
                if (auto thread_s_ptr = thread_w_ptr.lock())
                {
                    auto next_list = get_all_thread(thread_s_ptr);
                    leaf_list.insert(leaf_list.end(), next_list.begin(), next_list.end());
                }
            }
        }
        return leaf_list;
    }
    static ThreadChain::S_Ptr get_root(ThreadChain::S_Ptr thread)
    {
        if(thread==nullptr)
            return nullptr;
        if (thread->m_father.get()){
            return get_root(thread->m_father);
        }
        return thread;
    }

    static bool is_root(ThreadChain::S_Ptr thread){
        return thread!=nullptr && thread->m_father.get()==nullptr;
    }

    int get_thread_state()
    {
        return m_thread_state.load(std::memory_order_seq_cst);
    }

    int get_work_state()
    {
        return m_work_state.load(std::memory_order_seq_cst);
    }

    int get_play_state() //只有顶层调用
    {
        return m_play_state;
    }

    void set_play_state(int state) //只有顶层调用
    {
        m_play_state = state;
    }
    void add_thread(ThreadChain::S_Ptr child); 
    void notify();
    void sync();
    void init_0();
    void start_0();
    void stop_0();
    void play_0();
    void pause_0();
    void seek_0(long position);
private: 
    void uninit_0();
    void try_stop_0();
    static void seek_0_l(std::list<ThreadChain::S_Ptr> leaf_list, long position);
protected:
    virtual bool pause_condition() { return false; }
    virtual bool stop_condition() { return false; }
    virtual bool notify_condition() { return true; }
    virtual long get_wait_time(){return 0;}
    virtual void deal_neg_wait_time(){}
    virtual int init() {return 0;}
    virtual void uninit();
    virtual void start();
    virtual void stop();
    virtual void try_stop();
    virtual void play();
    virtual void pause();
    virtual void seek(long position) {}
    virtual void thread_func();
    virtual int work_func() { return 0; } //-1异常，0正常处理，1处理结束
    virtual void clean_func() {} //清理资源
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
    std::mutex m_play_mutex;            // 全局播放锁,只有顶层可以使用
    int m_play_state;                   // 播放状态，只有顶层可以使用
    static thread_local std::list<ThreadChain::S_Ptr> m_all_thread; //所有线程的强引用
};

#endif