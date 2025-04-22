#include "ThreadChain.h"
thread_local std::list<ThreadChain::S_Ptr> ThreadChain::m_all_thread; // 所有线程的强引用

void ThreadChain::add_thread(ThreadChain::S_Ptr child)
{
    child->m_level = m_level + 1; // 层级+1
    m_child_list.push_back(child);
    m_all_thread.push_back(child); // 保存一个强引用，防止对象析构
    child->m_father = shared_from_this();
    child->add_listener(this); // 父设置到子的消息链上
}

void ThreadChain::notify()
{
    std::unique_lock<std::recursive_mutex> lock(m_thread_mutex);
    int thread_state = m_thread_state.exchange(THREAD_RUNNING, std::memory_order_seq_cst);
    if (thread_state == THREAD_WAITING) // 实际唤醒
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

void ThreadChain::try_stop_0() // 只设置根节点
{
    auto root = get_root(shared_from_this());
    root->try_stop();
}

void ThreadChain::start()
{
    ThreadChain::S_Ptr thread = shared_from_this();
    m_thread_state.store(THREAD_RUNNING, std::memory_order_seq_cst); // 启动的时候设置线程状态
    m_work_state.store(IS_WORKING, std::memory_order_seq_cst);       // 启动的时候设置工作状态
    m_thread = new std::thread([thread]()
                               { thread->thread_func(); });
}

void ThreadChain::thread_func()
{
    if (thread_init() == 0)
    {
        while (true)
        {
            m_thread_state.store(THREAD_PAUSE, std::memory_order_seq_cst);
            int work_state = m_work_state.load(std::memory_order_seq_cst);
            if (work_state == IS_STOPPED || work_state == IS_STOPING)
            {
                break;
            }
            if (pause_condition(work_state))
            {
                if (work_state == IS_WORKING && stop_condition() && m_father && m_father->get_work_state() == IS_STOPING)
                {
                    // 当前可以设置成正在停止状态
                    m_work_state.compare_exchange_strong(work_state, IS_STOPING, std::memory_order_seq_cst);
                    continue;
                }
                else
                {
                    long wait_time=get_wait_time();
                    if (wait_time > 0)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                    }
                    else if (wait_time == 0)
                    {
                        std::unique_lock<std::recursive_mutex> lock(m_thread_mutex);
                        int thread_state = m_thread_state.load(std::memory_order_seq_cst);
                        if (thread_state == THREAD_PAUSE)
                        {
                            m_thread_state.store(THREAD_WAITING, std::memory_order_seq_cst);
                            m_thread_cond.wait(lock);
                        }
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
    clean_func();
}

int ThreadChain::stop_0()
{
    auto root = get_root(shared_from_this());
    return root->stop_0();
}

int ThreadChain::init(long system_time) 
{
    set_delegate(this);
    do_init(system_time);
    return 0;
}

void ThreadChain::seek(long pts_time,long system_time) 
{
    do_seek(pts_time,system_time);
}

void ThreadChain::play(long system_time)
{
    int expected = m_work_state.load(std::memory_order_seq_cst);
    if (expected == IS_PAUSED)
    {
        if (m_work_state.compare_exchange_strong(expected, IS_WORKING, std::memory_order_seq_cst))
        {
            do_play(system_time);
            notify();
        }
    }
}

void ThreadChain::pause()
{
    int expected = m_work_state.load(std::memory_order_seq_cst);
    if (expected == IS_WORKING)
    {
        if(m_work_state.compare_exchange_strong(expected, IS_PAUSED, std::memory_order_seq_cst)){
            do_pause();  
        }
    }
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
            do_try_stop();
            notify();
            break;
        }
    }
}

void ThreadChain::stop()
{
    int work_state=m_work_state.exchange(IS_STOPPED, std::memory_order_seq_cst);
    if(work_state!=IS_STOPPED){
        do_stop();
        notify();
    }
}

void ThreadChain::uninit()
{
    m_all_thread.remove(shared_from_this()); // 去除强引用，可能发生析构
    do_uninit();
}

std::list<ThreadChain::S_Ptr> ThreadChain::get_all_thread(ThreadChain::S_Ptr thread)
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

ThreadChain::S_Ptr ThreadChain::get_root(ThreadChain::S_Ptr thread)
{
    if(thread==nullptr)
        return nullptr;
    if (thread->m_father.get()){
        return get_root(thread->m_father);
    }
    return thread;
}

bool ThreadChain::is_root(ThreadChain::S_Ptr thread){
    return thread!=nullptr && thread->m_father.get()==nullptr;
}
