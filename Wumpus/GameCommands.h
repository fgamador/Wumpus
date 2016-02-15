#pragma once

#include <set>
using namespace std;

class GameCommands
{
public:
    virtual set<Event> MovePlayer(int room) = 0;
    virtual set<Event> Replay() = 0;
    virtual set<Event> Restart() = 0;
};
