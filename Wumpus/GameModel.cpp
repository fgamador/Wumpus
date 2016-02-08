#include "GameModel.h"

GameModel::GameModel(IRandomSource& randomSource)
    : m_randomSource(&randomSource)
{
}

void GameModel::RandomInit()
{
    m_playerRoom = m_randomSource->NextInt(1, 20);
    m_wumpusRoom = m_randomSource->NextInt(1, 20);
}

void GameModel::SetPlayerRoom(int room)
{
    if (room < 1 || room > 20)
        throw NoSuchRoomException();

    m_playerRoom = room;
}

void GameModel::SetWumpusRoom(int room)
{
    if (room < 1 || room > 20)
        throw NoSuchRoomException();

    m_wumpusRoom = room;
}

void GameModel::MovePlayer(int room)
{
    if (room < 1 || room > 20)
        throw NoSuchRoomException();

    if (!m_map.AreConnected(m_playerRoom, room))
        throw RoomsNotConnectedException();

    m_playerRoom = room;
}

int GameModel::GetPlayerRoom() const
{
    return m_playerRoom;
}

ints3 GameModel::GetPlayerConnectedRooms() const
{
    return m_map.GetConnectedRooms(m_playerRoom);
}

bool GameModel::WumpusAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_wumpusRoom);
}

int GameModel::GetWumpusRoom() const
{
    return m_wumpusRoom;
}
