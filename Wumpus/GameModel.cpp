#include "GameModel.h"

void GameModel::SetPlayerRoom(int room)
{
    if (room < 1 || room > 20)
        throw NoSuchRoomException();

    m_playerRoom = room;
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
