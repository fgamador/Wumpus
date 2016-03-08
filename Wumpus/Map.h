#pragma once

#include "Exceptions.h"
#include "stdtypes.h"

class Map
{
public:
    Map();

    ints3 GetConnectedRooms(int room) const;
    bool AreConnected(int room1, int room2) const;

private:
    // Room numbers are one-based. We leave the zero row empty.
    array<ints3, 21> m_connections;
};
