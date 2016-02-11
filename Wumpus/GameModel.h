#pragma once

#include "Event.h"
#include "GameCommands.h"
#include "GameMap.h"
#include "IRandomSource.h"
#include "PlayerState.h"
#include "RoomsNotConnectedException.h"

class GameModel : public GameCommands, public PlayerState
{
public:
    GameModel(IRandomSource& randomSource);

    void RandomInit();
    void SetPlayerRoom(int room);
    void SetWumpusRoom(int room);
    set<Event> MovePlayer(int room) override;

    bool PlayerAlive() const override;
    int GetPlayerRoom() const override;
    ints3 GetPlayerConnectedRooms() const override;
    bool WumpusAdjacent() const override;

    int GetWumpusRoom() const;

private:
    IRandomSource* m_randomSource;
    GameMap m_map;
    bool m_playerAlive = true;
    int m_playerRoom;
    int m_wumpusRoom;
};
