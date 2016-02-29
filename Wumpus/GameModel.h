#pragma once

#include "ArrowAlreadyPreparedException.h"
#include "ArrowDoubleBackException.h"
#include "ArrowPathLengthException.h"
#include "Event.h"
#include "GameCommands.h"
#include "GameMap.h"
#include "RandomSource.h"
#include "OutOfArrowsException.h"
#include "PlayerState.h"
#include "PlayerDeadException.h"
#include "RoomsNotConnectedException.h"

typedef array<int, 2> ints2;

class GameModel : public GameCommands, public PlayerState
{
public:
    static const int MaxArrows = 5;

    GameModel(RandomSource& randomSource);

    void SetPlayerRoom(int room);
    void SetWumpusRoom(int room);
    void SetBatRooms(int room1, int room2);
    void SetPitRooms(int room1, int room2);

    eventvec RandomPlacements() override;
    eventvec MovePlayer(int room) override;
    void PrepareArrow(int pathLength) override;
    eventvec MoveArrow(int room) override;
    eventvec Replay() override;
    eventvec Restart() override;

    bool PlayerAlive() const override;
    int GetPlayerRoom() const override;
    ints3 GetPlayerConnectedRooms() const override;
    bool WumpusAdjacent() const override;
    bool BatsAdjacent() const override;
    bool PitAdjacent() const override;
    bool WumpusAlive() const override;

    int GetWumpusRoom() const;
    ints2 GetBatRooms() const;
    ints2 GetPitRooms() const;
    int GetArrowsRemaining() const;
    int GetArrowMovesRemaining() const;

private:
    void Init();
    void ValidateMovePlayer(int room);
    eventvec PlacePlayer(int room);
    eventvec BumpedWumpusInBatRoom();
    eventvec BumpedWumpusInPitRoom();
    eventvec BumpedWumpus();
    eventvec BatSnatch();
    eventvec FellInPit();
    void ValidateMoveArrow(int room);
    eventvec ShotSelf();
    eventvec ShotWumpus();
    eventvec MissedWumpus();
    eventvec MoveWumpus();

private:
    RandomSource* m_randomSource;
    GameMap m_map;
    int m_initialPlayerRoom;
    int m_initialWumpusRoom;

    bool m_playerAlive;
    bool m_wumpusAlive;
    int m_playerRoom;
    int m_wumpusRoom;
    ints2 m_batRooms;
    ints2 m_pitRooms;
    int m_arrowsRemaining;
    int m_arrowMovesRemaining;
    int m_arrowRoom;
    int m_prevArrowRoom;
};
