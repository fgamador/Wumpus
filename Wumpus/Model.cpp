#include "Model.h"

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

Model::Model(RandomSource& randomSource)
    : m_randomSource(&randomSource)
{
    Init();
}

void Model::Init()
{
    m_playerAlive = true;
    m_wumpusRoom = true;
    m_arrowsRemaining = MaxArrows;
    m_arrowMovesRemaining = 0;
}

eventvec Model::RandomPlacements()
{
    m_initialPlayerRoom = m_randomSource->NextInt(1, 20);
    m_wumpusRoom = m_initialWumpusRoom = m_randomSource->NextInt(1, 20);
    m_batRooms[0] = m_randomSource->NextInt(1, 20);
    m_batRooms[1] = m_randomSource->NextInt(1, 20);
    m_pitRooms[0] = m_randomSource->NextInt(1, 20);
    m_pitRooms[1] = m_randomSource->NextInt(1, 20);
    return PlacePlayer(m_initialPlayerRoom);
}

void Model::SetPlayerRoom(int room)
{
    ValidateRoom(room);
    m_playerRoom = room;
}

void Model::SetWumpusRoom(int room)
{
    ValidateRoom(room);
    m_wumpusRoom = room;
}

void Model::SetBatRooms(int room1, int room2)
{
    ValidateRoom(room1);
    ValidateRoom(room2);
    m_batRooms = { room1, room2 };
}

void Model::SetPitRooms(int room1, int room2)
{
    ValidateRoom(room1);
    ValidateRoom(room2);
    m_pitRooms = { room1, room2 };
}

eventvec Model::MovePlayer(int room)
{
    ValidateMovePlayer(room);
    return PlacePlayer(room);
}

void Model::ValidateMovePlayer(int room)
{
    if (!m_playerAlive)
        throw PlayerDeadException();
    ValidateRoom(room);
    if (!m_map.AreConnected(m_playerRoom, room))
        throw RoomsNotConnectedException();
}

eventvec Model::PlacePlayer(int room)
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

eventvec Model::BumpedWumpusInBatRoom()
{
    eventvec events = Append({ Event::BumpedWumpus }, BatSnatch());
    if (!m_playerAlive)
        return events;

    return Append(events, MoveWumpus());
}

eventvec Model::BumpedWumpusInPitRoom()
{
    eventvec events = BumpedWumpus();
    if (!m_playerAlive)
        return events;

    return Append(events, FellInPit());
}

eventvec Model::BumpedWumpus()
{
    return Append({ Event::BumpedWumpus }, MoveWumpus());
}

eventvec Model::MoveWumpus()
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

eventvec Model::BatSnatch()
{
    return Append({ Event::BatSnatch }, PlacePlayer(m_randomSource->NextInt(1, 20)));
}

eventvec Model::FellInPit()
{
    m_playerAlive = false;
    return { Event::FellInPit };
}

void Model::PrepareArrow(int pathLength)
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

eventvec Model::MoveArrow(int room)
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

void Model::ValidateMoveArrow(int room)
{
    if (m_arrowMovesRemaining <= 0)
        throw ArrowPathLengthException();
    ValidateRoom(room);
    if (!m_map.AreConnected(m_arrowRoom, room))
        throw RoomsNotConnectedException();
    if (room == m_prevArrowRoom)
        throw ArrowDoubleBackException();
}

eventvec Model::ShotSelf()
{
    m_playerAlive = false;
    return { Event::ShotSelf };
}

eventvec Model::ShotWumpus()
{
    m_wumpusAlive = false;
    return { Event::KilledWumpus };
}

eventvec Model::MissedWumpus()
{
    return Append({ Event::MissedWumpus }, MoveWumpus());
}

eventvec Model::Replay()
{
    Init();
    m_wumpusRoom = m_initialWumpusRoom;
    return PlacePlayer(m_initialPlayerRoom);
}

eventvec Model::Restart()
{
    Init();
    return RandomPlacements();
}

bool Model::PlayerAlive() const
{
    return m_playerAlive;
}

int Model::GetPlayerRoom() const
{
    return m_playerRoom;
}

ints3 Model::GetPlayerConnectedRooms() const
{
    return m_map.GetConnectedRooms(m_playerRoom);
}

bool Model::WumpusAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_wumpusRoom);
}

bool Model::BatsAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_batRooms[0]) || m_map.AreConnected(m_playerRoom, m_batRooms[1]);
}

bool Model::PitAdjacent() const
{
    return m_map.AreConnected(m_playerRoom, m_pitRooms[0]) || m_map.AreConnected(m_playerRoom, m_pitRooms[1]);
}

bool Model::WumpusAlive() const
{
    return m_wumpusAlive;
}

int Model::GetWumpusRoom() const
{
    return m_wumpusRoom;
}

ints2 Model::GetBatRooms() const
{
    return m_batRooms;
}

ints2 Model::GetPitRooms() const
{
    return m_pitRooms;
}

int Model::GetArrowsRemaining() const
{
    return m_arrowsRemaining;
}

int Model::GetArrowMovesRemaining() const
{
    return m_arrowMovesRemaining;
}
