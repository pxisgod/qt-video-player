#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H
#include <list>
#include <memory>

enum ThreadMsgType
{
    INIT_DONE,
    SWITCH_PLAYING,
    SWITCH_PAUSE,
    SWITCH_STOP
};

typedef struct ThreadMsg
{
    int m_msg_type;
    long m_msg_time;
} ThreadMsg;

template<typename Event>
class EventNotifier;

template<typename Event>
class EventListener{
public:
    EventListener(EventNotifier<Event>*delegate):delegate(delegate){
    }
    EventListener(){
    }
    virtual ~EventListener(){
    }
    void deal_event_0(Event event){
        // 处理事件
        if(delegate){
            delegate->emit_event(event);
        }else{
            deal_event(event);
        }
    }
    void set_delegate(EventNotifier<Event>* delegate){
        this->delegate=delegate;
    }
    EventNotifier<Event> *get_delegate(){
        return delegate;
    }
protected:
    virtual void deal_event(Event event){};
private:
    EventNotifier<Event>*  delegate;
};

template<typename Event>
class EventNotifier{
public:
    EventNotifier(){}
    virtual ~EventNotifier(){}
    void emit_event(Event event){
        for(auto event_listener:m_listener_list){
            event_listener->deal_event_0(event);
        }
    }
    void add_listener(EventListener<Event>* event_listener){
        m_listener_list.push_back(event_listener);
    }
    void remove_listener(EventListener<Event> * event_listener){
        m_listener_list.remove(event_listener);
    }
private:
    std::list<EventListener<Event>*> m_listener_list;
};
#endif