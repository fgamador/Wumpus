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

eventvec GameModel::RandomInit()
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
        return BumpedWumpusThenBatSnatch();

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

eventvec GameModel::BumpedWumpusThenBatSnatch()
{
    eventvec events = { Event::BumpedWumpus };
    eventvec snatchEvents = BatSnatch();
    events.insert(events.end(), snatchEvents.begin(), snatchEvents.end());
    eventvec moveEvents = MoveWumpus();
    events.insert(events.end(), moveEvents.begin(), moveEvents.end());
    return events;
}

eventvec GameModel::BumpedWumpusInPitRoom()
{
    eventvec events = BumpedWumpus();
    if (PlayerAlive())
    {
        eventvec pitEvents = FellInPit();
        events.insert(events.end(), pitEvents.begin(), pitEvents.end());
    }
    return events;
}

eventvec GameModel::BumpedWumpus()
{
    eventvec events = MoveWumpus();
    events.insert(events.begin(), Event::BumpedWumpus);
    return events;
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
    eventvec events = PlacePlayer(m_randomSource->NextInt(1, 20));
    events.insert(events.begin(), Event::BatSnatch);
    return events;
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
    m_arrowMovesRemaining = 0; // TODO test
    return{ Event::ShotSelf };
}

eventvec GameModel::ShotWumpus()
{
    m_wumpusAlive = false;
    m_arrowMovesRemaining = 0;
    return { Event::KilledWumpus };
}

eventvec GameModel::MissedWumpus()
{
    eventvec events = MoveWumpus();
    events.insert(events.begin(), Event::MissedWumpus);
    return events;
}

eventvec GameModel::Replay()
{
    m_playerAlive = true;
    m_wumpusRoom = m_initialWumpusRoom;
    m_arrowsRemaining = MaxArrows; // TODO test
    return PlacePlayer(m_initialPlayerRoom);
}

eventvec GameModel::Restart()
{
    m_playerAlive = true;
    m_arrowsRemaining = MaxArrows; // TODO test
    return RandomInit();
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
