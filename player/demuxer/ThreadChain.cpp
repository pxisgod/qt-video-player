#include "ThreadChain.h"

void ThreadChain::add_thread(ThreadChain::S_Ptr child)
{
    child->m_level = m_level + 1; // 层级+1
    m_child_list.push_back(child);
    m_all_thread.push_back(child); // 保存一个强引用，防止对象析构
    child->m_father = shared_from_this();
}

void ThreadChain::notify()
{
    int thread_state = m_thread_state.exchange(THREAD_RUNNING, std::memory_order_seq_cst);
    if (thread_state == THREAD_PAUSE)
    {
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
        m_play_state = PLAYING;//默认初始状态
        // 从父向子遍历
        auto leaf_list = get_all_thread(shared_from_this());
        leaf_list.sort();
        for (auto iter = leaf_list.begin(); iter != leaf_list.end(); iter++)
        {
            (*iter)->start();
            (*iter)->uninit(); // 线程中保存了强引用，可以清除原来的强引用
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

void ThreadChain::play_0() // 只有顶层可以使用
{
    if (is_root(shared_from_this()))
    {
        std::lock_guard<std::mutex> lock(m_play_mutex);
        if (m_play_state == PAUSE)
        {
            m_play_state = PLAYING;
            // 从父向子遍历
            auto leaf_list = get_all_thread(shared_from_this());
            leaf_list.sort();
            for (auto iter = leaf_list.begin(); iter != leaf_list.end(); iter++)
            {
                (*iter)->play();
            }
        }
    }
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
        std::lock_guard<std::mutex> lock(thread->m_rsc_mutex); // 获取资源锁，这个资源锁会互相嵌套
        thread->seek(position);
        leaf_list.pop_front();
        seek_0_l(leaf_list, position);
    }
}

void ThreadChain::uninit()
{
    m_all_thread.remove(shared_from_this()); // 去除强引用，可能发生析构
}

void ThreadChain::start()
{
    ThreadChain::S_Ptr thread = shared_from_this();
    m_thread_state.store(THREAD_RUNNING, std::memory_order_seq_cst);
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
    int expected = m_thread_state.load(std::memory_order_seq_cst);
    if (expected == THREAD_PAUSE)
    {
        if (m_thread_state.compare_exchange_strong(expected, THREAD_RUNNING))
        {
            notify();
        }
    }
}

void ThreadChain::pause()
{
    int expected = m_thread_state.load(std::memory_order_seq_cst);
    if (expected == THREAD_RUNNING)
    {
        m_thread_state.compare_exchange_strong(expected, THREAD_PAUSE);
    }
}

void ThreadChain::thread_func()
{
    while (true)
    {
        m_thread_state.store(THREAD_PAUSE, std::memory_order_seq_cst);
        int work_state = m_work_state.load(std::memory_order_seq_cst);
        if (work_state == IS_STOPPED || work_state == IS_STOPING)
        {
            break;
        }
        if (work_state == IS_PAUSED || pause_condition())
        {
            if (work_state == IS_WORKING && stop_condition() && m_father->get_work_state() == IS_STOPING)
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
                    std::unique_lock<std::recursive_mutex> lock(m_thread_mutex);
                    m_thread_cond.wait_for(lock,std::chrono::milliseconds(wait_time));
                }else if(wait_time==0){
                    std::unique_lock<std::recursive_mutex> lock(m_thread_mutex);
                    m_thread_cond.wait(lock);
                }else{
                    do_some_things();
                    continue;
                }
            }
        }
        m_thread_state.store(THREAD_RUNNING);
        {
            std::lock_guard<std::mutex> lock(m_rsc_mutex);
            int result = work_func();
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
    // 等待子线程结束
    for (auto thread : m_child_list)
    {
        auto thread_ptr = thread.lock();
        if (thread_ptr != nullptr)
        {
            thread_ptr->notify(); // 如果是stopping状态需要通知
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
    {
        // 清理工作
        std::lock_guard<std::mutex> lock(m_rsc_mutex);
        clean_func();
    }
    // 修改全局状态
    if (is_root(shared_from_this())) // 只有顶层可以使用
    {
        // 根节点
        std::lock_guard<std::mutex> lock(m_play_mutex);
        m_play_state = STOPPED; // 修改状态为停止状态
    }
}
