#include "CommandInterpreter.h"

#include <sstream>

#include "Msg.h"

class CommandInterpreter::State
{
public:
    virtual void Input(string input, CommandInterpreter& interp) const = 0;
    virtual void OutputStandardMessage(CommandInterpreter& interp) const = 0;

protected:
    void OutputEvents(const eventvec& events, CommandInterpreter& interp) const
    {
        for (Event event : events)
        {
            switch (event)
            {
            case Event::BatSnatch:
                interp.Output(Msg::BatSnatch);
                break;
            case Event::BumpedWumpus:
                interp.Output(Msg::BumpedWumpus);
                break;
            case Event::EatenByWumpus:
                interp.Output(Msg::WumpusGotYou);
                break;
            }
        }
    }
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

class CommandInterpreter::AwaitingReplayState : public State
{
public:
    void Input(string input, CommandInterpreter& interp) const override;
    void OutputStandardMessage(CommandInterpreter& interp) const override;
};

CommandInterpreter::InitialState CommandInterpreter::Initial;
CommandInterpreter::AwaitingCommandState CommandInterpreter::AwaitingCommand;
CommandInterpreter::AwaitingMoveRoomState CommandInterpreter::AwaitingMoveRoom;
CommandInterpreter::AwaitingReplayState CommandInterpreter::AwaitingReplay;

CommandInterpreter::CommandInterpreter(GameCommands& commands, const PlayerState& playerState)
    : m_commands(commands)
    , m_playerState(playerState)
    , m_state(&Initial)
{
}

void CommandInterpreter::Run(istream& in, ostream& out)
{
    string input;
    while (!in.eof())
    {
        strvec output = Input(input);
        for (size_t i = 0; i < output.size(); ++i)
        {
            if (i > 0)
                out << endl;
            out << output[i];
        }
        getline(in, input);
    }
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
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
}

void CommandInterpreter::AwaitingCommandState::OutputStandardMessage(CommandInterpreter& interp) const
{
    interp.Output(Msg::ShootOrMove);
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
        eventvec events = interp.m_commands.MovePlayer(stoi(input));
        OutputEvents(events, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
        return;
    }
    catch (const GameException&)
    {
        interp.Output(Msg::Impossible);
        OutputStandardMessage(interp);
        return;
    }

    if (interp.m_playerState.PlayerAlive())
    {
        Initial.OutputStandardMessage(interp);
        interp.SetState(AwaitingCommand);
    }
    else
    {
        interp.Output(Msg::YouLose);
        interp.SetState(AwaitingReplay);
    }
}

void CommandInterpreter::AwaitingMoveRoomState::OutputStandardMessage(CommandInterpreter& interp) const
{
    interp.Output(Msg::WhereTo);
}

void CommandInterpreter::AwaitingReplayState::Input(string input, CommandInterpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
    }
    else if (input == "Y" || input == "y")
    {
        interp.m_commands.Replay();
        interp.Output(Msg::HuntTheWumpus);
        Initial.OutputStandardMessage(interp);
        interp.SetState(AwaitingCommand);
    }
    else if (input == "N" || input == "n")
    {
        interp.m_commands.Restart();
        interp.Output(Msg::HuntTheWumpus);
        Initial.OutputStandardMessage(interp);
        interp.SetState(AwaitingCommand);
    }
    else
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
}

void CommandInterpreter::AwaitingReplayState::OutputStandardMessage(CommandInterpreter& interp) const
{
    interp.Output(Msg::SameSetup);
}

void CommandInterpreter::OutputPlayerState()
{
    if (m_playerState.WumpusAdjacent())
        Output(Msg::SmellWumpus);
    if (m_playerState.BatsAdjacent())
        Output(Msg::BatsNearby);

    ostringstream out1;
    out1 << Msg::YouAreInRoom << m_playerState.GetPlayerRoom();
    Output(out1.str());

    ints3 connected = m_playerState.GetPlayerConnectedRooms();
    ostringstream out2;
    out2 << Msg::TunnelsLeadTo << connected[0] << " " << connected[1] << " " << connected[2];
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
