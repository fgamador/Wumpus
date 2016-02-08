#include "GameMap.h"

GameMap::GameMap()
{
    m_connections[1] = { 2, 5, 8 };
    m_connections[2] = { 1, 3, 10 };
    m_connections[3] = { 2, 4, 12 };
    m_connections[4] = { 3, 5, 14 };
    m_connections[5] = { 1, 4, 6 };
    m_connections[6] = { 5, 7, 15 };
    m_connections[7] = { 6, 8, 17 };
    m_connections[8] = { 1, 7, 9 };
    m_connections[9] = { 8, 10, 18 };
    m_connections[10] = { 2, 9, 11 };
    m_connections[11] = { 10, 12, 19 };
    m_connections[12] = { 3, 11, 13 };
    m_connections[13] = { 12, 14, 20 };
    m_connections[14] = { 4, 13, 15 };
    m_connections[15] = { 6, 14, 16 };
    m_connections[16] = { 15, 17, 20 };
    m_connections[17] = { 7, 16, 18 };
    m_connections[18] = { 9, 17, 19 };
    m_connections[19] = { 11, 18, 20 };
    m_connections[20] = { 13, 16, 19 };
}

ints3 GameMap::GetConnectedRooms(int room) const
{
    if (room < 1 || room > 20)
        throw NoSuchRoomException();

    return m_connections[room];
}

bool GameMap::AreConnected(int room1, int room2) const
{
    ints3 connectedRooms = m_connections[room1];
    return find(connectedRooms.begin(), connectedRooms.end(), room2) != connectedRooms.end();
}
