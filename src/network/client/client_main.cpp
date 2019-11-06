//
//  main.cpp
//  dirtyjam
//
//  Created by Eugene Sturm on 1/25/19.
//

#include "Socket.hpp"
#include "Connection.hpp"
#include <iostream>
#include <SDL.h>
#import <SDL_syswm.h>
#include "Game.hpp"
#include <MetalBackend.h>
#include <RenderBackend.h>
#include <RenderDevice.h>

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cout << "SDL Init Failed:" << SDL_GetError();
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);
    
    uint32_t windowWidth = 1200;
    uint32_t windowHeight = 800;
    SDL_Window* window = SDL_CreateWindow("dirty", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        std::cout << "SDL could not create window:" << SDL_GetError();
        return EXIT_FAILURE;
    }
    
    gfx::RenderBackend* renderBackend = new gfx::MetalBackend();
    gfx::RenderDevice* device = renderBackend->getRenderDevice();
    
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    NSWindow* nsWindow = (NSWindow*)info.info.cocoa.window;
    
    gfx::SwapchainDesc swapchainDesc;
    swapchainDesc.format = gfx::PixelFormat::BGRA8Unorm;
    swapchainDesc.height = windowWidth;
    swapchainDesc.width = windowHeight;
    
    gfx::Swapchain* swapchain = renderBackend->createSwapchainForWindow(swapchainDesc, device, (void*)nsWindow);
                                        
    Socket socket;
    ConnectionPtr connection = socket.connect("localhost", 44951);
    
    connection->waitForConnectionState(ConnectionState::Connected);
    
    std::cout << "Connected\n";
    
    connection->queueOutgoing(Packet(AuthMessage()));
    Packet incoming;
    while ( (incoming = connection->waitForIncoming()).messageType() != MessageType::AuthResponse) {
        std::cout << to_string(incoming.messageType()) << std::endl;
    }
    
    std::cout << "Authed\n";
    
    connection->queueOutgoing(Packet(LoginMessage("sturm")));
    
    while ( (incoming = connection->waitForIncoming()).messageType() != MessageType::LoginResponse) {
        std::cout << to_string(incoming.messageType()) << std::endl;
    }
    
    std::cout << "Logged in\n";
    
    std::array<float, 3> position;
    incoming.read(position.data(), sizeof(float) * 3);
    
    
    Game game;
    game.initialize(device, swapchain, connection);
    
    bool shouldQuit = false;
    std::vector<SDL_Event> events;
    std::vector<Packet> packets;
    double dt = 0;
    while (shouldQuit == false) {
        events.clear();
        packets.clear();
        
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                shouldQuit = true;
                break;
            }
            events.emplace_back(std::move(e));
        }
        
        connection->drainIncomingQueue(&packets);
        game.update(events, packets, dt);
    }
    
    game.shutdown();
    
    SDL_DestroyWindow(window);
    return EXIT_SUCCESS;
}


//switch (e.type) {
//    case SDL_QUIT: {
//        shouldQuit = true;
//        break;
//    }
//    case SDL_WINDOWEVENT: {
//        break;
//    }
//    case SDL_KEYDOWN:
//    case SDL_KEYUP:
//    case SDL_MOUSEMOTION:
//    case SDL_MOUSEBUTTONDOWN:
//    case SDL_CONTROLLERBUTTONDOWN:
//    case SDL_CONTROLLERBUTTONUP:
//    case SDL_CONTROLLERAXISMOTION: {
//        break;
//    }
//}
