#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H
#include <list>
#include <memory>
template<typename Event>
class EventListener{
    void emitEvent(Event event){
        for(auto eventListener:m_EventListenerList){
            eventListener->emitEvent(event);
        }
    }
    void addListener(std::shared_ptr<EventListener> eventListener){
        m_EventListenerList.push_back(eventListener);
    }
private:
    std::list<std::shared_ptr<EventListener<Event>>> m_EventListenerList;
};
#endif