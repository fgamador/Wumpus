#pragma once

#include "Event.h"
#include "GameCommands.h"
#include "GameMap.h"
#include "IRandomSource.h"
#include "PlayerState.h"
#include "PlayerDeadException.h"
#include "RoomsNotConnectedException.h"

typedef array<int, 2> ints2;

class GameModel : public GameCommands, public PlayerState
{
public:
    GameModel(IRandomSource& randomSource);

    void RandomInit();
    void SetPlayerRoom(int room);
    void SetWumpusRoom(int room);
    void SetBatsRooms(int room1, int room2);
    void SetPitRooms(int room1, int room2);
    eventvec MovePlayer(int room) override;
    eventvec Replay() override;
    eventvec Restart() override;

    bool PlayerAlive() const override;
    int GetPlayerRoom() const override;
    ints3 GetPlayerConnectedRooms() const override;
    bool WumpusAdjacent() const override;
    bool BatsAdjacent() const override;
    bool PitAdjacent() const override;

    int GetWumpusRoom() const;
    int GetBatsRoom1() const;
    int GetBatsRoom2() const;
    ints2 GetPitRooms() const;

private:
    eventvec PlacePlayer(int room);

private:
    IRandomSource* m_randomSource;
    GameMap m_map;
    int m_initialPlayerRoom;

    bool m_playerAlive = true;
    int m_playerRoom;
    int m_wumpusRoom;
    int m_batsRoom1;
    int m_batsRoom2;
    ints2 m_pitRooms;
};
