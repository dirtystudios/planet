
#pragma once

#include <algorithm>
#include <vector>
#include <stdint.h>
#include "DrawItem.h"
#include "RenderDevice.h"
#include "CommandBuffer.h"

class RenderQueue {
private:
    typedef std::pair<uint32_t, const gfx::DrawItem*> DrawItemPair;
    std::vector<DrawItemPair> _items;
    gfx::CommandBuffer* _cmdbuf;
public:
    RenderQueue(gfx::CommandBuffer* cmdbuf) : _cmdbuf(cmdbuf) {}

    void AddDrawItem(uint32_t key, const gfx::DrawItem* item) {
        _items.push_back(std::make_pair(key, item));
    }


    void Submit(gfx::RenderDevice* device) {
        Sort();
        for (const DrawItemPair& p : _items) {
            _cmdbuf->DrawItem(p.second);
        }
        device->Submit({ _cmdbuf });
    }

private:
    void Sort() {
        std::sort(begin(_items), end(_items),
            [](const DrawItemPair& p1, const DrawItemPair& p2) { return p1.first < p2.first; });
    }
};
