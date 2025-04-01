#ifndef EVNET_LISTENER_H
#define EVNET_LISTENER_H
#include <list>
#include <memory>
template<typename Event>
class EventListener{
public:
    void emit(Event event);
    void addListener(std::shared_ptr<EventListener<Event>> eventListener);
private:
    std::list<std::shared_ptr<EventListener<Event>>> m_EventListenerList;
};
#endif