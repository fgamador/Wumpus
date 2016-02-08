#pragma once

#include <array>

using namespace std;

typedef array<int, 3> ints3;

class PlayerState
{
public:
    virtual int GetPlayerRoom() const = 0;
    virtual ints3 GetPlayerConnectedRooms() const = 0;
    virtual bool WumpusAdjacent() const = 0;
};
