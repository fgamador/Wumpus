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
    virtual void OutputStateMessage(Interpreter& interp) const = 0;

protected:
};

class Interpreter::InitialState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingCommandState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingMoveRoomState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    void MovePlayer(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowPathLengthState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    void PrepareArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowRoomState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    void MoveArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingReplayState : public State
{
public:
    void Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    void StartGame(const eventvec& events, Interpreter& interp) const;
};

class Interpreter::EndState : public State
{
public:
    void Input(string input, Interpreter& interp) const override {}
    void OutputStateMessage(Interpreter& interp) const override {}
};

Interpreter::InitialState Interpreter::Initial;
Interpreter::AwaitingCommandState Interpreter::AwaitingCommand;
Interpreter::AwaitingMoveRoomState Interpreter::AwaitingMoveRoom;
Interpreter::AwaitingArrowPathLengthState Interpreter::AwaitingArrowPathLength;
Interpreter::AwaitingArrowRoomState Interpreter::AwaitingArrowRoom;
Interpreter::AwaitingReplayState Interpreter::AwaitingReplay;
Interpreter::EndState Interpreter::End;

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
    m_state->OutputStateMessage(*this);
    return m_output;
}

void Interpreter::InitialState::Input(string input, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);
    interp.Output("");

    if (input == "")
    {
        OutputStateMessage(interp);
        interp.SetState(AwaitingCommand);
        return;
    }

    eventvec events = interp.m_commands.RandomPlacements();
    interp.OutputEvents(events);
    interp.CheckPlayerAlive();
}

void Interpreter::InitialState::OutputStateMessage(Interpreter& interp) const
{
    interp.OutputPlayerState();
    interp.Output("");
}

void Interpreter::AwaitingCommandState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        return;
    }

    if (input == "M" || input == "m")
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
    }
}

void Interpreter::AwaitingCommandState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::ShootOrMove);
}

void Interpreter::AwaitingMoveRoomState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        return;
    }

    try
    {
        MovePlayer(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
    }
    catch (const GameException&)
    {
        interp.Output(Msg::Impossible);
    }
}

void Interpreter::AwaitingMoveRoomState::MovePlayer(const string& input, Interpreter& interp) const
{
    eventvec events = interp.m_commands.MovePlayer(stoi(input));
    interp.Output("");
    interp.OutputEvents(events);
    interp.CheckPlayerAlive();
}

void Interpreter::AwaitingMoveRoomState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::WhereTo);
}

void Interpreter::AwaitingArrowPathLengthState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        return;
    }

    try
    {
        PrepareArrow(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
    }
    catch (const ArrowPathLengthException&)
    {
        interp.Output(Msg::Impossible);
    }
}

void Interpreter::AwaitingArrowPathLengthState::PrepareArrow(const string& input, Interpreter& interp) const
{
    interp.m_commands.PrepareArrow(stoi(input));
    interp.SetState(AwaitingArrowRoom);
}

void Interpreter::AwaitingArrowPathLengthState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::NumberOfRooms);
}

void Interpreter::AwaitingArrowRoomState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        return;
    }

    try
    {
        MoveArrow(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
    }
    catch (const ArrowDoubleBackException&)
    {
        interp.Output(Msg::NotThatCrooked);
    }
}

void Interpreter::AwaitingArrowRoomState::MoveArrow(const string& input, Interpreter& interp) const
{
    eventvec events = interp.m_commands.MoveArrow(stoi(input));
    if (events.empty())
    {
        return;
    }

    interp.Output("");
    interp.OutputEvents(events);

    if (!interp.m_playerState.WumpusAlive())
    {
        interp.Output(Msg::GetYouNextTime);
        interp.Output(Msg::Exit);
        interp.SetState(End);
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

void Interpreter::AwaitingArrowRoomState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::RoomNumber);
}

void Interpreter::AwaitingReplayState::Input(string input, Interpreter& interp) const
{
    if (input == "")
    {
        return;
    }

    if (input == "Y" || input == "y")
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
    }
}

void Interpreter::AwaitingReplayState::StartGame(const eventvec& events, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);
    interp.Output("");
    interp.OutputEvents(events);
    interp.CheckPlayerAlive();
}

void Interpreter::AwaitingReplayState::OutputStateMessage(Interpreter& interp) const
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
