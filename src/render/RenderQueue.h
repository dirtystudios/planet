
#pragma once

#include <algorithm>
#include <stdint.h>
#include <vector>
#include "CommandBuffer.h"
#include "DrawItem.h"
#include "RenderDevice.h"
#include "StateGroup.h"

class RenderQueue {
private:
    typedef std::pair<uint32_t, const gfx::DrawItem*> DrawItemPair;
    std::vector<DrawItemPair> _items;
    gfx::CommandBuffer*       _cmdbuf;

public:
    // temporary
    const gfx::StateGroup* defaults{nullptr};

    RenderQueue(gfx::CommandBuffer* cmdbuf) : _cmdbuf(cmdbuf) {}

    void AddDrawItem(uint32_t key, const gfx::DrawItem* item) { _items.push_back(std::make_pair(key, item)); }

    void Submit(gfx::RenderDevice* device) {
        Sort();
        for (const DrawItemPair& p : _items) {
            _cmdbuf->DrawItem(p.second);
        }
        device->Submit({_cmdbuf});
    }

private:
    void Sort() {
        std::sort(begin(_items), end(_items), [](const DrawItemPair& p1, const DrawItemPair& p2) { return p1.first < p2.first; });
    }
};
