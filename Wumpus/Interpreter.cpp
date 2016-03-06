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

private:
    void MovePlayer(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowPathLengthState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;

private:
    void PrepareArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowRoomState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;

private:
    void MoveArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingReplayState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStandardMessage(Interpreter& interp) const override;

private:
    void StartGame(const eventvec& events, Interpreter& interp) const;
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
    interp.Output("");

    if (input == "")
    {
        OutputStandardMessage(interp);
        interp.SetState(AwaitingCommand);
        return;
    }

    eventvec events = interp.m_commands.RandomPlacements();
    interp.OutputEvents(events);
    interp.CheckPlayerAlive();
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
        MovePlayer(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
    catch (const GameException&)
    {
        interp.Output(Msg::Impossible);
        OutputStandardMessage(interp);
    }
}

void Interpreter::AwaitingMoveRoomState::MovePlayer(const string& input, Interpreter& interp) const
{
    eventvec events = interp.m_commands.MovePlayer(stoi(input));
    interp.Output("");
    interp.OutputEvents(events);
    interp.CheckPlayerAlive();
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
        PrepareArrow(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
    catch (const ArrowPathLengthException&)
    {
        interp.Output(Msg::Impossible);
        OutputStandardMessage(interp);
    }
}

void Interpreter::AwaitingArrowPathLengthState::PrepareArrow(const string& input, Interpreter& interp) const
{
    interp.m_commands.PrepareArrow(stoi(input));
    interp.SetState(AwaitingArrowRoom);
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
        MoveArrow(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
    catch (const ArrowDoubleBackException&)
    {
        interp.Output(Msg::NotThatCrooked);
        OutputStandardMessage(interp);
    }
}

void Interpreter::AwaitingArrowRoomState::MoveArrow(const string& input, Interpreter& interp) const
{
    eventvec events = interp.m_commands.MoveArrow(stoi(input));
    if (events.empty())
    {
        OutputStandardMessage(interp);
        return;
    }

    interp.Output("");
    interp.OutputEvents(events);

    if (!interp.m_playerState.WumpusAlive())
    {
        interp.Output(Msg::GetYouNextTime);
        interp.Output(Msg::Exit);
        return;
    }
    else if (interp.m_playerState.GetArrowsRemaining() == 0)
    {
        interp.Output(Msg::OutOfArrows);
        interp.Output(Msg::YouLose);
        interp.SetState(AwaitingReplay);
        return;
    }

    interp.CheckPlayerAlive();
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
        StartGame(events, interp);
    }
    else if (input == "N" || input == "n")
    {
        eventvec events = interp.m_commands.Restart();
        StartGame(events, interp);
    }
    else
    {
        interp.Output(Msg::Huh);
        OutputStandardMessage(interp);
    }
}

void Interpreter::AwaitingReplayState::StartGame(const eventvec& events, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);
    interp.Output("");
    interp.OutputEvents(events);
    interp.CheckPlayerAlive();
    // TODO lame; we're already in AwaitingReplayState, so msg not automatically output
    if (!interp.m_playerState.PlayerAlive())
        OutputStandardMessage(interp);
}

void Interpreter::AwaitingReplayState::OutputStandardMessage(Interpreter& interp) const
{
    interp.Output(Msg::SameSetup);
}

void Interpreter::CheckPlayerAlive()
{
    if (m_playerState.PlayerAlive())
    {
        OutputPlayerState();
        Output("");
        SetState(AwaitingCommand);
    }
    else
    {
        Output(Msg::YouLose);
        SetState(AwaitingReplay);
    }
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
