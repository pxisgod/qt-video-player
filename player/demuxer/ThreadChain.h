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

struct TypeCompare
{
    bool operator()(ThreadChain a, ThreadChain b)
    {
        return a.level < b.level;
    }
};

struct TypeCompareReverse
{
    bool operator()(ThreadChain a, ThreadChain b)
    {
        return b.level < a.level;
    }
};

class ThreadChain : public std::enable_shared_from_this<ThreadChain>
{
public:
    using S_Ptr = std::shared_ptr<ThreadChain>;
    using W_Ptr = std::weak_ptr<ThreadChain>;

    explicit ThreadChain()
    {
        level = 0;
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
        return level < b.level;
    }

    void add_thread(ThreadChain::S_Ptr child);
    void notify();
    void sync();
    void init_0();
    void uninit_0();
    void start_0();
    void stop_0();
    void try_stop_0();
    void play_0();
    void pause_0();
    void seek_0(long position);
    static void seek_0_l(std::list<ThreadChain::S_Ptr> leaf_list, long position);
    virtual bool pause_condition() { return false; }
    virtual bool stop_condition() { return false; }
    virtual int init() {}
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

    int get_thread_state()
    {
        return m_thread_state.load(std::memory_order_seq_cst);
    }

    int get_work_state()
    {
        return m_work_state.load(std::memory_order_seq_cst);
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
        if (thread->m_father.get())
        {
            return get_root(thread->m_father);
        }
        return thread;
    }

    static int get_play_state()
    {
        return m_play_state;
    }

    static void set_play_state(int state)
    {
        m_play_state = state;
    }

public:
    int level; // 在树中的层级

private:
    std::list<ThreadChain::W_Ptr> m_child_list;
    ThreadChain::S_Ptr m_father;
    std::thread *m_thread;
    std::atomic<int> m_thread_state;           // 线程状态
    std::atomic<int> m_work_state;             // 工作状态
    std::recursive_mutex m_thread_mutex;       // 暂停条件锁
    std::condition_variable_any m_thread_cond; // 暂停条件
    std::mutex m_rsc_mutex;                    // 资源锁
    static std::mutex m_play_mutex;            // 全局播放锁
    static int m_play_state;                   // 播放状态
    static std::list<ThreadChain::S_Ptr> m_all_thread; //所有线程的强引用
};

#endif