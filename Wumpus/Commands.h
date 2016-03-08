#pragma once

#include "Event.h"

using eventvec = vector<Event>;

class Commands
{
public:
    virtual eventvec RandomPlacements() = 0;
    virtual eventvec MovePlayer(int room) = 0;
    virtual void PrepareArrow(int pathLength) = 0;
    virtual eventvec MoveArrow(int room) = 0;
    virtual eventvec Replay() = 0;
    virtual eventvec Restart() = 0;
};
