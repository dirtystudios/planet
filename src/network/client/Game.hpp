//
//  Game.hpp
//  dirtyclient
//
//  Created by Eugene Sturm on 2/3/19.
//

#pragma once

#include <memory>
#include <SDL.h>
#include "Packet.hpp"
#include <vector>
#include <RenderDevice.h>
#include <Swapchain.h>

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class Game
{
private:
    ConnectionPtr _connection { nullptr };
    gfx::RenderDevice* _device { nullptr };
    gfx::Swapchain* _swapchain { nullptr };
    
    gfx::RenderPassId _renderPassId { gfx::NULL_ID };
    
    bool _windowHasFocus;
public:
    void initialize(gfx::RenderDevice* device, gfx::Swapchain* swapchain, ConnectionPtr& connection);
    void update(const std::vector<SDL_Event>& systemEvents, const std::vector<Packet>& packets, double dt);
    void shutdown();
};
