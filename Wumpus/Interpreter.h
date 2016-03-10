#pragma once

#include <iostream>
#include "Model.h"
#include "stdtypes.h"

class Interpreter
{
public:
    static const string Randomize;
    static const map<Event, string> EventMsgs;

    Interpreter(Commands& commands, const PlayerState& playerState);

    void Run(istream& in, ostream& out);
    strvec Input(string input);

private:
    class State;
    class InitialState;
    class AwaitingCommandState;
    class AwaitingMoveRoomState;
    class AwaitingArrowPathLengthState;
    class AwaitingArrowRoomState;
    class AwaitingReplayState;
    class EndState;

private:
    const State& CheckAndOutputPlayerState(const eventvec& events);
    const State& WumpusDied();
    const State& PlayerDied();
    const State& OutOfArrows();
    const State& PlayerStillAlive();
    void OutputEvents(const eventvec& events);
    void OutputAdjacentHazards();
    void OutputPlayerLocation();
    void Output(const string& str);

private:
    Commands& m_commands;
    const PlayerState& m_playerState;
    const State* m_state;
    strvec m_output;

    static InitialState Initial;
    static AwaitingCommandState AwaitingCommand;
    static AwaitingMoveRoomState AwaitingMoveRoom;
    static AwaitingArrowPathLengthState AwaitingArrowPathLength;
    static AwaitingArrowRoomState AwaitingArrowRoom;
    static AwaitingReplayState AwaitingReplay;
    static EndState End;
};
