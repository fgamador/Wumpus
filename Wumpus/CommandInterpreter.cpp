#include "CommandInterpreter.h"

#include <sstream>

class CommandInterpreter::State
{
public:
    virtual void Input(string input, CommandInterpreter& interp) const = 0;
    virtual void OutputStandardMessage(CommandInterpreter& interp) const = 0;
};

class CommandInterpreter::InitialState : public State
{
public:
    void Input(string input, CommandInterpreter& interp) const override;
    void OutputStandardMessage(CommandInterpreter& interp) const override;
};

class CommandInterpreter::AwaitingCommandState : public State
{
public:
    void Input(string input, CommandInterpreter& interp) const override;
    void OutputStandardMessage(CommandInterpreter& interp) const override;
};

class CommandInterpreter::AwaitingMoveRoomState : public State
{
public:
    void Input(string input, CommandInterpreter& interp) const override;
    void OutputStandardMessage(CommandInterpreter& interp) const override;
};

CommandInterpreter::InitialState CommandInterpreter::Initial;
CommandInterpreter::AwaitingCommandState CommandInterpreter::AwaitingCommand;
CommandInterpreter::AwaitingMoveRoomState CommandInterpreter::AwaitingMoveRoom;

CommandInterpreter::CommandInterpreter(GameCommands& commands, const PlayerState& playerState)
    : m_commands(commands)
    , m_playerState(playerState)
    , m_state(&Initial)
{
}

strvec CommandInterpreter::Input(string input)
{
    m_output.clear();
    const State* prevState = m_state;
    m_state->Input(input, *this);
    if (m_state != prevState)
        m_state->OutputStandardMessage(*this);
    return m_output;
}

void CommandInterpreter::InitialState::Input(string input, CommandInterpreter& interp) const
{
    OutputStandardMessage(interp);
    interp.SetState(AwaitingCommand);
}

void CommandInterpreter::InitialState::OutputStandardMessage(CommandInterpreter& interp) const
{
    interp.OutputPlayerState();
    interp.Output("");
}

void CommandInterpreter::AwaitingCommandState::Input(string input, CommandInterpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
    }
    else if (input == "M" || input == "m")
    {
        interp.SetState(AwaitingMoveRoom);
    }
    else
    {
        interp.Output("HUH?");
        OutputStandardMessage(interp);
    }
}

void CommandInterpreter::AwaitingCommandState::OutputStandardMessage(CommandInterpreter& interp) const
{
    interp.Output("SHOOT OR MOVE (S-M)? ");
}

void CommandInterpreter::AwaitingMoveRoomState::Input(string input, CommandInterpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
        return;
    }

    try
    {
        interp.m_commands.MovePlayer(stoi(input));
    }
    catch (const exception&)
    {
        interp.Output("HUH?");
        OutputStandardMessage(interp);
        return;
    }
    catch (const GameException&)
    {
        interp.Output("IMPOSSIBLE");
        OutputStandardMessage(interp);
        return;
    }

    Initial.OutputStandardMessage(interp);
    interp.SetState(AwaitingCommand);
}

void CommandInterpreter::AwaitingMoveRoomState::OutputStandardMessage(CommandInterpreter& interp) const
{
    interp.Output("WHERE TO? ");
}

void CommandInterpreter::OutputPlayerState()
{
    ostringstream out1;
    out1 << "YOU ARE IN ROOM " << m_playerState.GetPlayerRoom();
    Output(out1.str());

    ints3 connected = m_playerState.GetPlayerConnectedRooms();
    ostringstream out2;
    out2 << "TUNNELS LEAD TO " << connected[0] << " " << connected[1] << " " << connected[2];
    Output(out2.str());
}

void CommandInterpreter::Output(const string& str)
{
    m_output.push_back(str);
}

void CommandInterpreter::SetState(const State& state)
{
    m_state = &state;
}
