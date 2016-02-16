#include "GameModel.h"

GameModel::GameModel(IRandomSource& randomSource)
    : m_randomSource(&randomSource)
{
}

void GameModel::RandomInit()
{
    m_playerRoom = m_initialPlayerRoom = m_randomSource->NextInt(1, 20);
    m_wumpusRoom = m_randomSource->NextInt(1, 20);
    // TODO check for initial events?
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

eventvec GameModel::MovePlayer(int room)
{
    if (!m_playerAlive)
        throw PlayerDeadException();

    if (room < 1 || room > 20)
        throw NoSuchRoomException();

    if (!m_map.AreConnected(m_playerRoom, room))
        throw RoomsNotConnectedException();

    m_playerRoom = room;

    if (m_playerRoom == m_wumpusRoom)
    {
        if (m_randomSource->NextInt(1, 4) == 4)
        {
            m_wumpusRoom = m_map.GetConnectedRooms(m_wumpusRoom)[0]; // TODO random room
            return{ Event::BumpedWumpus };
        }

        m_playerAlive = false;
        return{ Event::BumpedWumpus, Event::EatenByWumpus };
    }

    return {};
}

eventvec GameModel::Replay()
{
    m_playerRoom = m_initialPlayerRoom;
    // TODO restore wumpus
    // TODO check for initial events
    return {};
}

eventvec GameModel::Restart()
{
    RandomInit();
    // TODO check for initial events
    return{};
}

bool GameModel::PlayerAlive() const
{
    return m_playerAlive;
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
