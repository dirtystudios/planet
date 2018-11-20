#pragma once

struct BaseEvent {
    virtual ~BaseEvent() {}
protected:
    static size_t getNextType() {
        static size_t type_count = 0;
        return type_count++;
    }
};

template <typename EventType>
struct Event : BaseEvent {
    static size_t type() {
        static size_t t_type = BaseEvent::getNextType();
        return t_type;
    }
    Event(const EventType& event)
        : event_(event) {}
    const EventType& event_;
};