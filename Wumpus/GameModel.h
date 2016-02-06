#pragma once

#include "GameCommands.h"
#include "GameMap.h"
#include "PlayerState.h"
#include "RoomsNotConnectedException.h"

class GameModel : public GameCommands, public PlayerState
{
public:
    void SetPlayerRoom(int room);
    void MovePlayer(int room) override;

    int GetPlayerRoom() const override;
    ints3 GetPlayerConnectedRooms() const override;

private:
    GameMap m_map;
    int m_playerRoom;
};
