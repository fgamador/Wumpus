#pragma once

#include <vector>
#include <string>
#include "GameModel.h"

using namespace std;

typedef vector<string> strvec;

class CommandInterpreter
{
public:
    CommandInterpreter(GameCommands& commands, const PlayerState& playerState);

    strvec Input(string input);

private:
    class State;
    class InitialState;
    class AwaitingCommandState;
    class AwaitingMoveRoomState;

private:
    void OutputPlayerState();
    void Output(const string& str);
    void SetState(const State& state);

private:
    GameCommands& m_commands;
    const PlayerState& m_playerState;
    const State* m_state;
    strvec m_output;

    static InitialState Initial;
    static AwaitingCommandState AwaitingCommand;
    static AwaitingMoveRoomState AwaitingMoveRoom;
};
