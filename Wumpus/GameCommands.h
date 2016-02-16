#pragma once

#include <vector>
using namespace std;
typedef vector<Event> eventvec;

class GameCommands
{
public:
    virtual eventvec MovePlayer(int room) = 0;
    virtual eventvec Replay() = 0;
    virtual eventvec Restart() = 0;
};
