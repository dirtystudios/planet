//
//  Player.hpp
//  dirtyserver
//
//  Created by Eugene Sturm on 2/5/19.
//

#pragma once

#include <array>
#include <string>

class Session;
class CharacterDatabase;

struct PlayerCreateInfo
{
    uint64_t guid { 0 };
    std::string name;
};

class Player
{
private:
    uint64_t _guid{ 0 };
    Session* _session { nullptr };
    std::string _name;
public:
    Player(const PlayerCreateInfo& createInfo, Session* session);

    uint64_t guid() { return _guid; }
    Session* session() { return _session; }
    const std::string& name() const { return _name; }

    inline bool operator==(const Player& o) {
        return _guid == o._guid;
    }

    inline bool operator!=(const Player& o) {
        return _guid != o._guid;
    }
};
