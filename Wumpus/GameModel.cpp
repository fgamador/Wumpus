#include "GameModel.h"

namespace
{
    void ValidateRoom(int room)
    {
        if (room < 1 || room > 20)
            throw NoSuchRoomException();
    }

    eventvec Append(const eventvec& events1, const eventvec& events2)
    {
        eventvec events = events1;
        events.insert(events.end(), events2.begin(), events2.end());
        return events;
    }
}

GameModel::GameModel(RandomSource& randomSource)
    : m_randomSource(&randomSource)
{
    Init();
}

void GameModel::Init()
{
    m_playerAlive = true;
    m_wumpusRoom = true;
    m_arrowsRemaining = MaxArrows;
    m_arrowMovesRemaining = 0;
}

eventvec GameModel::RandomPlacements()
{
    m_initialPlayerRoom = m_randomSource->NextInt(1, 20);
    m_wumpusRoom = m_initialWumpusRoom = m_randomSource->NextInt(1, 20);
    m_batRooms[0] = m_randomSource->NextInt(1, 20);
    m_batRooms[1] = m_randomSource->NextInt(1, 20);
    m_pitRooms[0] = m_randomSource->NextInt(1, 20);
    m_pitRooms[1] = m_randomSource->NextInt(1, 20);
    return PlacePlayer(m_initialPlayerRoom);
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

void GameModel::SetBatRooms(int room1, int room2)
{
    ValidateRoom(room1);
    ValidateRoom(room2);
    m_batRooms = { room1, room2 };
}

void GameModel::SetPitRooms(int room1, int room2)
{
    ValidateRoom(room1);
    ValidateRoom(room2);
    m_pitRooms = { room1, room2 };
}

eventvec GameModel::MovePlayer(int room)
{
    ValidateMovePlayer(room);
    return PlacePlayer(room);
}

void GameModel::ValidateMovePlayer(int room)
{
    if (!m_playerAlive)
        throw PlayerDeadException();
    ValidateRoom(room);
    if (!m_map.AreConnected(m_playerRoom, room))
        throw RoomsNotConnectedException();
}

eventvec GameModel::PlacePlayer(int room)
{
    m_playerRoom = room;

    bool inWumpusRoom = (m_playerRoom == m_wumpusRoom);
    bool inBatRoom = (m_playerRoom == m_batRooms[0] || m_playerRoom == m_batRooms[1]);
    bool inPitRoom = (m_playerRoom == m_pitRooms[0] || m_playerRoom == m_pitRooms[1]);

    if (inWumpusRoom && inBatRoom)
        return BumpedWumpusInBatRoom();

    if (inWumpusRoom && inPitRoom)
        return BumpedWumpusInPitRoom();

    if (inWumpusRoom)
        return BumpedWumpus();

    if (inBatRoom)
        return BatSnatch();

    if (inPitRoom)
        return FellInPit();

    return {};
}

eventvec GameModel::BumpedWumpusInBatRoom()
{
    eventvec events = Append({ Event::BumpedWumpus }, BatSnatch());
    if (!m_playerAlive)
        return events;

    return Append(events, MoveWumpus());
}

eventvec GameModel::BumpedWumpusInPitRoom()
{
    eventvec events = BumpedWumpus();
    if (!m_playerAlive)
        return events;

    return Append(events, FellInPit());
}

eventvec GameModel::BumpedWumpus()
{
    return Append({ Event::BumpedWumpus }, MoveWumpus());
}

eventvec GameModel::MoveWumpus()
{
    ints3 connectedRooms = m_map.GetConnectedRooms(m_wumpusRoom);
    unsigned roomIndex = static_cast<unsigned>(m_randomSource->NextInt(0, 3));
    if (roomIndex < connectedRooms.size())
        m_wumpusRoom = connectedRooms[roomIndex];

    if (m_wumpusRoom == m_playerRoom)
    {
        m_playerAlive = false;
        return { Event::EatenByWumpus };
    }

    return {};
}

eventvec GameModel::BatSnatch()
{
    return Append({ Event::BatSnatch }, PlacePlayer(m_randomSource->NextInt(1, 20)));
}

eventvec GameModel::FellInPit()
{
    m_playerAlive = false;
    return { Event::FellInPit };
}

void GameModel::PrepareArrow(int pathLength)
{
    if (m_arrowMovesRemaining > 0)
        throw ArrowAlreadyPreparedException();
    if (m_arrowsRemaining == 0)
        throw OutOfArrowsException();
    if (pathLength < 1 || pathLength > 5)
        throw ArrowPathLengthException();

    m_arrowsRemaining--;
    m_arrowMovesRemaining = pathLength;
    m_arrowRoom = m_prevArrowRoom = m_playerRoom;
}

eventvec GameModel::MoveArrow(int room)
{
    ValidateMoveArrow(room);

    m_arrowMovesRemaining--;
    m_prevArrowRoom = m_arrowRoom;
    m_arrowRoom = room;

    if (m_arrowRoom == m_playerRoom)
        return ShotSelf();

    if (m_arrowRoom == m_wumpusRoom)
        return ShotWumpus();

    if (m_arrowMovesRemaining == 0)
        return MissedWumpus();

    return {};
}

void GameModel::ValidateMoveArrow(int room)
{
    if (m_arrowMovesRemaining <= 0)
        throw ArrowPathLengthException();
    ValidateRoom(room);
    if (!m_map.AreConnected(m_arrowRoom, room))
        throw RoomsNotConnectedException();
    if (room == m_prevArrowRoom)
        throw ArrowDoubleBackException();
}

eventvec GameModel::ShotSelf()
{
    m_playerAlive = false;
    return { Event::ShotSelf };
}

eventvec GameModel::ShotWumpus()
{
    m_wumpusAlive = false;
    return { Event::KilledWumpus };
}

eventvec GameModel::MissedWumpus()
{
    return Append({ Event::MissedWumpus }, MoveWumpus());
}

eventvec GameModel::Replay()
{
    Init();
    m_wumpusRoom = m_initialWumpusRoom;
    return PlacePlayer(m_initialPlayerRoom);
}

eventvec GameModel::Restart()
{
    Init();
    return RandomPlacements();
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
    return m_map.AreConnected(m_playerRoom, m_batRooms[0]) || m_map.AreConnected(m_playerRoom, m_batRooms[1]);
}

bool GameModel::PitAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_pitRooms[0]) || m_map.AreConnected(m_playerRoom, m_pitRooms[1]);
}

bool GameModel::WumpusAlive() const
{
    return m_wumpusAlive;
}

int GameModel::GetWumpusRoom() const
{
    return m_wumpusRoom;
}

ints2 GameModel::GetBatRooms() const
{
    return m_batRooms;
}

ints2 GameModel::GetPitRooms() const
{
    return m_pitRooms;
}

int GameModel::GetArrowsRemaining() const
{
    return m_arrowsRemaining;
}

int GameModel::GetArrowMovesRemaining() const
{
    return m_arrowMovesRemaining;
}
