#pragma once

#include <set>
using namespace std;

class GameCommands
{
public:
    virtual set<Event> MovePlayer(int room) = 0;
};
