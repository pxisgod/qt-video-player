#include "ThreadChain.h"
thread_local std::list<ThreadChain::S_Ptr> ThreadChain::m_all_thread; // 所有线程的强引用

void ThreadChain::add_thread(ThreadChain::S_Ptr child)
{
    child->m_level = m_level + 1; // 层级+1
    m_child_list.push_back(child);
    m_all_thread.push_back(child); // 保存一个强引用，防止对象析构
    child->m_father = shared_from_this();
}

void ThreadChain::notify()
{
    std::unique_lock<std::recursive_mutex> lock(m_thread_mutex); //占用锁，并且在锁上后判断有没有其他线程在wait,防止发生伪唤醒
    int thread_state = m_thread_state.exchange(THREAD_RUNNING, std::memory_order_seq_cst);
    if (thread_state == THREAD_PAUSE)
    {
        notify_debug();
        m_thread_cond.notify_one();
    }
}

void ThreadChain::sync()
{
    if (m_thread != nullptr)
    {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
}

void ThreadChain::init_0() // 只有顶层可以使用
{
    if (is_root(shared_from_this()))
    {
        if (init() != 0)
        {
            uninit_0(); // 全部去除初始化
        }
    }
}

void ThreadChain::uninit_0() // 只有顶层可以使用
{
    // 从子向父遍历
    auto leaf_list = get_all_thread(shared_from_this());
    leaf_list.sort();
    for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
    {
        (*iter)->uninit(); // 线程中保存了强引用，可以清除原来的强引用
    }
}

void ThreadChain::start_0()
{
    if (is_root(shared_from_this()))
    {
        std::lock_guard<std::mutex> lock(m_play_mutex);
        if (m_play_state == STOPPED)
        {
            m_play_state = PLAYING; // 默认初始状态
            // 从子向父遍历
            auto leaf_list = get_all_thread(shared_from_this());
            leaf_list.sort();
            for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
            {
                (*iter)->start();
                (*iter)->uninit(); // 线程中保存了强引用，可以清除原来的强引用
            }
        }
    }
}

void ThreadChain::play_0() // 只有顶层可以使用
{
    if (is_root(shared_from_this()))
    {
        std::lock_guard<std::mutex> lock(m_play_mutex);
        if (m_play_state == PAUSE)
        {
            m_play_state = PLAYING;
            // 从子向父遍历
            auto leaf_list = get_all_thread(shared_from_this());
            leaf_list.sort();
            for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
            {
                (*iter)->play();
            }
        }
    }
}

void ThreadChain::stop_0()
{
    // 从子向父遍历
    auto leaf_list = get_all_thread(get_root(shared_from_this()));
    leaf_list.sort();
    for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
    {
        (*iter)->stop();
    }
}

void ThreadChain::try_stop_0() // 只设置根节点
{
    auto root = get_root(shared_from_this());
    root->try_stop();
}

void ThreadChain::pause_0() // 只有顶层可以使用
{
    if (is_root(shared_from_this()))
    {
        std::lock_guard<std::mutex> lock(m_play_mutex);
        if (m_play_state == PLAYING)
        {
            m_play_state = PAUSE;
            // 从子向父遍历
            auto leaf_list = get_all_thread(shared_from_this());
            leaf_list.sort();
            for (auto iter = leaf_list.rbegin(); iter != leaf_list.rend(); iter++)
            {
                (*iter)->pause();
            }
        }
    }
}

void ThreadChain::seek_0(long position) // 只有顶层可以使用
{
    if (is_root(shared_from_this()))
    {
        std::lock_guard<std::mutex> lock(m_play_mutex);
        {
            if (m_play_state == PLAYING || m_play_state == PAUSE)
            {
                // 从子向父遍历
                auto leaf_list = get_all_thread(shared_from_this());
                leaf_list.sort();
                seek_0_l(leaf_list, position);
            }
        }
    }
}

void ThreadChain::seek_0_l(std::list<ThreadChain::S_Ptr> leaf_list, long position)
{
    if (!leaf_list.empty())
    {
        ThreadChain::S_Ptr thread = leaf_list.front();
        std::lock_guard<std::mutex> lock(thread->m_rsc_mutex); // 反向获取所有资源锁，全部获取后才会开始调用seek方法
        leaf_list.pop_front();
        seek_0_l(leaf_list, position);
        thread->seek(position); // 正向遍历seek方法
    }
}

void ThreadChain::uninit()
{
    m_all_thread.remove(shared_from_this()); // 去除强引用，可能发生析构
}

void ThreadChain::start()
{
    ThreadChain::S_Ptr thread = shared_from_this();
    m_thread_state.store(THREAD_RUNNING, std::memory_order_seq_cst); // 启动的时候设置线程状态
    m_work_state.store(IS_WORKING, std::memory_order_seq_cst);       // 启动的时候设置工作状态
    m_thread = new std::thread([thread]()
                               { thread->thread_func(); });
}

void ThreadChain::stop()
{
    m_work_state.store(IS_STOPPED, std::memory_order_seq_cst);
    notify();
}

void ThreadChain::try_stop()
{
    while (true)
    {
        int expected = m_work_state.load(std::memory_order_seq_cst);
        if (expected == IS_STOPING || expected == IS_STOPPED)
        {
            break;
        }
        if (m_work_state.compare_exchange_strong(expected, IS_STOPING, std::memory_order_seq_cst))
        {
            notify();
            break;
        }
    }
}

void ThreadChain::play()
{
    int expected = m_work_state.load(std::memory_order_seq_cst);
    if (expected == IS_PAUSED)
    {
        if (m_work_state.compare_exchange_strong(expected, IS_WORKING, std::memory_order_seq_cst))
        {
            notify();
        }
    }
}

void ThreadChain::pause()
{
    int expected = m_work_state.load(std::memory_order_seq_cst);
    if (expected == IS_WORKING)
    {
        m_work_state.compare_exchange_strong(expected, IS_PAUSED, std::memory_order_seq_cst);
    }
}

void ThreadChain::thread_func()
{
    if (thread_init() == 0)
    {
        while (true)
        {
            int work_state = m_work_state.load(std::memory_order_seq_cst);
            if (work_state == IS_STOPPED || work_state == IS_STOPING)
            {
                break;
            }
            if (pause_condition() || work_state == IS_PAUSED)
            {
                if (work_state == IS_WORKING && stop_condition() && m_father && m_father->get_work_state() == IS_STOPING)
                {
                    // 当前可以设置成正在停止状态
                    m_work_state.compare_exchange_strong(work_state, IS_STOPING, std::memory_order_seq_cst);
                    continue;
                }
                else
                {
                    long wait_time = get_wait_time();
                    if (wait_time > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                    }
                    else if (wait_time == 0)
                    {
                        std::unique_lock<std::recursive_mutex> lock(m_thread_mutex);
                        m_thread_state.store(THREAD_PAUSE, std::memory_order_seq_cst);
                        m_thread_cond.wait(lock);
                        continue;
                    }
                    else
                    {
                        deal_neg_wait_time();
                        continue;
                    }
                }
            }
            m_thread_state.store(THREAD_RUNNING);
            {
                std::lock_guard<std::mutex> lock(m_rsc_mutex);
                int result = work_func(); //-1异常，0正常处理，1处理结束
                if (result == -1)
                {
                    stop_0();
                }
                else if (result == 1)
                {
                    try_stop_0();
                }
            }
        }
    }
    else
    {
        stop_0();
    }
    // 等待子线程结束
    for (auto thread : m_child_list)
    {
        auto thread_ptr = thread.lock();
        if (thread_ptr != nullptr)
        {
            thread_ptr->notify();
        }
    }
    for (auto thread : m_child_list)
    {
        auto thread_ptr = thread.lock();
        if (thread_ptr != nullptr)
        {
            thread_ptr->sync();
        }
    }
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    clean_func();
    // 修改全局状态
    if (is_root(shared_from_this())) // 只有顶层可以使用
    {
        // 根节点
        std::lock_guard<std::mutex> lock(m_play_mutex);
        m_play_state = STOPPED; // 修改状态为停止状态
    }
}
