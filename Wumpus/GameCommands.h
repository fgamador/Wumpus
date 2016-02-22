#pragma once

#include <vector>
using namespace std;
typedef vector<Event> eventvec;
typedef vector<int> intvec;

class GameCommands
{
public:
    virtual eventvec MovePlayer(int room) = 0;
    virtual void PrepareArrow(int pathLength) = 0;
    virtual eventvec MoveArrow(int room) = 0;
    virtual eventvec Replay() = 0;
    virtual eventvec Restart() = 0;
};
