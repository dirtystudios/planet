#pragma once

#include <stdint.h>

namespace Guid {
    // todo: *actually* generate guids with this
    static uint64_t generateGuid() {
        static uint64_t guid = 0;
        return guid++;
    }
}
