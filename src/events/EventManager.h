#pragma once

#include "BaseEvent.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

class EventManager {
    template <class EventType>
    using call_type = std::function<bool(const EventType&)>;

    std::vector<std::vector<call_type<BaseEvent>>> m_subscribers;

public:
    template <typename EventType>
    struct CallbackWrapper {
        CallbackWrapper(call_type<EventType> callable)
            : m_callable(callable) {}

        bool operator()(const BaseEvent& event) { return m_callable(static_cast<const Event<EventType>&>(event).event_); }

        call_type<EventType> m_callable;
    };

    template <typename EventType>
    void subscribe(call_type<EventType> callable) {
        size_t type = Event<EventType>::type();
        if (type >= m_subscribers.size())
            m_subscribers.resize(type + 1);
        m_subscribers[type].push_back(CallbackWrapper<EventType>(callable));
    }

    template <typename EventType>
    void emit(const EventType& event) {
        size_t type = Event<EventType>::type();
        if (type >= m_subscribers.size())
            m_subscribers.resize(type + 1);

        Event<EventType> eventWrapper(event);
        for (auto& receiver : m_subscribers[type]) {
            if (!receiver(eventWrapper))
                break;
        }
    }
};