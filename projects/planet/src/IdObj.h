#pragma once

#include <stdint.h>

static uint64_t createId() {
    static uint64_t key = 0;
    return ++key;
}

class IdObj {
protected:
    const uint64_t _id{createId()};
};
