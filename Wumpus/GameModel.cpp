#include "GameModel.h"

namespace
{
    void ValidateRoom(int room)
    {
        if (room < 1 || room > 20)
            throw NoSuchRoomException();
    }
}

GameModel::GameModel(IRandomSource& randomSource)
    : m_randomSource(&randomSource)
{
}

void GameModel::RandomInit()
{
    m_playerRoom = m_initialPlayerRoom = m_randomSource->NextInt(1, 20);
    m_wumpusRoom = m_randomSource->NextInt(1, 20);
    m_batsRoom1 = m_randomSource->NextInt(1, 20);
    m_batsRoom2 = m_randomSource->NextInt(1, 20);
    m_pitRooms[0] = m_randomSource->NextInt(1, 20);
    m_pitRooms[1] = m_randomSource->NextInt(1, 20);
    // TODO check for initial events?
}

void GameModel::SetPlayerRoom(int room)
{
    ValidateRoom(room);
    m_playerRoom = room;
}

void GameModel::SetWumpusRoom(int room)
{
    ValidateRoom(room);
    m_wumpusRoom = room;
}

void GameModel::SetBatsRooms(int room1, int room2)
{
    ValidateRoom(room1);
    ValidateRoom(room2);
    m_batsRoom1 = room1;
    m_batsRoom2 = room2;
}

void GameModel::SetPitRooms(int room1, int room2)
{
    ValidateRoom(room1);
    ValidateRoom(room2);
    m_pitRooms = { room1, room2 };
}

eventvec GameModel::MovePlayer(int room)
{
    if (!m_playerAlive)
        throw PlayerDeadException();
    ValidateRoom(room);
    if (!m_map.AreConnected(m_playerRoom, room))
        throw RoomsNotConnectedException();

    return PlacePlayer(room);
}

eventvec GameModel::PlacePlayer(int room)
{
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

    if (m_playerRoom == m_batsRoom1 || m_playerRoom == m_batsRoom2)
    {
        eventvec events = PlacePlayer(m_randomSource->NextInt(1, 20));
        events.insert(events.begin(), Event::BatSnatch);
        return events;
    }

    if (m_playerRoom == m_pitRooms[0] || m_playerRoom == m_pitRooms[1])
    {
        m_playerAlive = false;
        return{ Event::FellInPit };
    }

    return{};
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

bool GameModel::BatsAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_batsRoom1) || m_map.AreConnected(m_playerRoom, m_batsRoom2);
}

bool GameModel::PitAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_pitRooms[0]) || m_map.AreConnected(m_playerRoom, m_pitRooms[1]);
}

int GameModel::GetWumpusRoom() const
{
    return m_wumpusRoom;
}

int GameModel::GetBatsRoom1() const
{
    return m_batsRoom1;
}

int GameModel::GetBatsRoom2() const
{
    return m_batsRoom2;
}

ints2 GameModel::GetPitRooms() const
{
    return m_pitRooms;
}
