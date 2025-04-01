#include "EventListener.h"

template<typename Event>
void EventListener<Event>::emit(Event event){
    for(auto eventListener:m_EventListenerList){
        eventListener->emit(event);
    }
}

template<typename Event>
void EventListener<Event>::addListener(std::shared_ptr<EventListener<Event>> eventListener){
    m_EventListenerList.push_back(eventListener);
}

