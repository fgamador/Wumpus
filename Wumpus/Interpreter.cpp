#include "Interpreter.h"

#include <map>
#include <sstream>

#include "Msg.h"

const string Interpreter::Randomize = "[Randomize]";

const map<Event, string> Interpreter::EventMsgs =
{
    { Event::BatSnatch, Msg::BatSnatch },
    { Event::BumpedWumpus, Msg::BumpedWumpus },
    { Event::EatenByWumpus, Msg::WumpusGotYou },
    { Event::FellInPit, Msg::FellInPit },
    { Event::KilledWumpus, Msg::GotTheWumpus },
    { Event::MissedWumpus, Msg::Missed }
};

class Interpreter::State
{
    static map<Event, string> eventMsgs;

public:
    virtual void Input(string input, Interpreter& interp) const = 0;
    virtual void OutputStandardMessage(Interpreter& interp) const = 0;

protected:
};

class Interpreter::InitialState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingCommandState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingMoveRoomState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingArrowPathLengthState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingArrowRoomState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingReplayState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;
};

Interpreter::InitialState Interpreter::Initial;
Interpreter::AwaitingCommandState Interpreter::AwaitingCommand;
Interpreter::AwaitingMoveRoomState Interpreter::AwaitingMoveRoom;
Interpreter::AwaitingArrowPathLengthState Interpreter::AwaitingArrowPathLength;
Interpreter::AwaitingArrowRoomState Interpreter::AwaitingArrowRoom;
Interpreter::AwaitingReplayState Interpreter::AwaitingReplay;

Interpreter::Interpreter(Commands& commands, const PlayerState& playerState)
    : m_commands(commands)
    , m_playerState(playerState)
    , m_state(&Initial)
{
}

void Interpreter::Run(istream& in, ostream& out)
{
    string input = Randomize;
    while (!in.eof())
    {
        strvec output = Input(input);
        for (size_t i = 0; i < output.size(); ++i)
        {
            if (output[i] == Msg::Exit)
                return;

            if (i > 0)
                out << endl;
            out << output[i];
        }
        getline(in, input);
    }
}

strvec Interpreter::Input(string input)
{
    m_output.clear();
    const State* prevState = m_state;
    m_state->Input(input, *this);
    if (m_state != prevState)
        m_state->OutputStandardMessage(*this);
    return m_output;
}

void Interpreter::InitialState::Input(string input, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);

    if (input == "")
    {
        interp.Output("");
        OutputStandardMessage(interp);
        interp.SetState(AwaitingCommand);
        return;
    }

    eventvec events = interp.m_commands.RandomPlacements();
    interp.Output("");
    interp.OutputEvents(events);

    if (interp.m_playerState.PlayerAlive())
    {
        OutputStandardMessage(interp);
        interp.SetState(AwaitingCommand);
    }
    else
    {
        interp.Output(Msg::YouLose);
        interp.SetState(AwaitingReplay);
    }
}

void Interpreter::InitialState::OutputStandardMessage(Interpreter& interp) const
{
    interp.OutputPlayerState();
    interp.Output("");
}

void Interpreter::AwaitingCommandState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
    }
    else if (input == "M" || input == "m")
    {
        interp.SetState(AwaitingMoveRoom);
    }
    else if (input == "S" || input == "s")
    {
        interp.SetState(AwaitingArrowPathLength);
    }
    else
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
}

void Interpreter::AwaitingCommandState::OutputStandardMessage(Interpreter& interp) const
{
    interp.Output(Msg::ShootOrMove);
}

void Interpreter::AwaitingMoveRoomState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
        return;
    }

    try
    {
        eventvec events = interp.m_commands.MovePlayer(stoi(input));
        interp.Output("");
        interp.OutputEvents(events);
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

void Interpreter::AwaitingMoveRoomState::OutputStandardMessage(Interpreter& interp) const
{
    interp.Output(Msg::WhereTo);
}

void Interpreter::AwaitingArrowPathLengthState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
        return;
    }

    try
    {
        interp.m_commands.PrepareArrow(stoi(input));
        interp.SetState(AwaitingArrowRoom);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
        return;
    }
    catch (const ArrowPathLengthException&)
    {
        interp.Output(Msg::Impossible);
        OutputStandardMessage(interp);
        return;
    }
}

void Interpreter::AwaitingArrowPathLengthState::OutputStandardMessage(Interpreter& interp) const
{
    interp.Output(Msg::NumberOfRooms);
}

void Interpreter::AwaitingArrowRoomState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
        return;
    }

    try
    {
        eventvec events = interp.m_commands.MoveArrow(stoi(input));
        if (!events.empty())
        {
            interp.Output("");
            interp.OutputEvents(events);
        }

        if (events.empty())
        {
            OutputStandardMessage(interp);
        }
        else if (!interp.m_playerState.WumpusAlive())
        {
            interp.Output(Msg::GetYouNextTime);
            interp.Output(Msg::Exit);
        }
        else if (interp.m_playerState.GetArrowsRemaining() == 0)
        {
            interp.Output(Msg::OutOfArrows);
            interp.Output(Msg::YouLose);
            interp.SetState(AwaitingReplay);
        }
        else if (!interp.m_playerState.PlayerAlive())
        {
            interp.Output(Msg::YouLose);
            interp.SetState(AwaitingReplay);
        }
        else
        {
            Initial.OutputStandardMessage(interp);
            interp.SetState(AwaitingCommand);
        }
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
        return;
    }
    catch (const ArrowDoubleBackException&)
    {
        interp.Output(Msg::NotThatCrooked);
        OutputStandardMessage(interp);
        return;
    }
}

void Interpreter::AwaitingArrowRoomState::OutputStandardMessage(Interpreter& interp) const
{
    interp.Output(Msg::RoomNumber);
}

void Interpreter::AwaitingReplayState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        OutputStandardMessage(interp);
    }
    else if (input == "Y" || input == "y")
    {
        eventvec events = interp.m_commands.Replay();
        interp.Output(Msg::HuntTheWumpus);
        interp.Output("");
        interp.OutputEvents(events);

        if (interp.m_playerState.PlayerAlive())
        {
            Initial.OutputStandardMessage(interp);
            interp.SetState(AwaitingCommand);
        }
        else
        {
            interp.Output(Msg::YouLose);
            OutputStandardMessage(interp);
        }
    }
    else if (input == "N" || input == "n")
    {
        eventvec events = interp.m_commands.Restart();
        interp.Output(Msg::HuntTheWumpus);
        interp.Output("");
        interp.OutputEvents(events);

        if (interp.m_playerState.PlayerAlive())
        {
            Initial.OutputStandardMessage(interp);
            interp.SetState(AwaitingCommand);
        }
        else
        {
            interp.Output(Msg::YouLose);
            OutputStandardMessage(interp);
        }
    }
    else
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
}

void Interpreter::AwaitingReplayState::OutputStandardMessage(Interpreter& interp) const
{
    interp.Output(Msg::SameSetup);
}

void Interpreter::Output(const string& str)
{
    m_output.push_back(str);
}

void Interpreter::OutputEvents(const eventvec& events)
{
    for (Event event : events)
        Output(EventMsgs.at(event));
}

void Interpreter::OutputPlayerState()
{
    if (m_playerState.WumpusAdjacent())
        Output(Msg::SmellWumpus);
    if (m_playerState.BatsAdjacent())
        Output(Msg::BatsNearby);
    if (m_playerState.PitAdjacent())
        Output(Msg::FeelDraft);

    ostringstream out1;
    out1 << Msg::YouAreInRoom << m_playerState.GetPlayerRoom();
    Output(out1.str());

    ints3 connected = m_playerState.GetPlayerConnectedRooms();
    ostringstream out2;
    out2 << Msg::TunnelsLeadTo << connected[0] << " " << connected[1] << " " << connected[2];
    Output(out2.str());
}

void Interpreter::SetState(const State& state)
{
    m_state = &state;
}
