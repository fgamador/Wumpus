#pragma once

#include <array>

using namespace std;

typedef array<int, 3> ints3;

class PlayerState
{
public:
    virtual bool PlayerAlive() const = 0;
    virtual int GetPlayerRoom() const = 0;
    virtual ints3 GetPlayerConnectedRooms() const = 0;
    virtual bool WumpusAdjacent() const = 0;
    virtual bool BatsAdjacent() const = 0;
    virtual bool PitAdjacent() const = 0;
    virtual bool WumpusAlive() const = 0;
};
