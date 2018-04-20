#pragma once

#include <stdint.h>

static uint64_t createId() {
    static uint64_t key = 0;
    return ++key;
}

class IdObj {
public:
    const uint64_t _id;
    IdObj(uint64_t id) : _id(id) {};
    IdObj() : _id(createId()) {};
};
