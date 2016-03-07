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
public:
    virtual const State& Input(string input, Interpreter& interp) const = 0;
    virtual void OutputStateMessage(Interpreter& interp) const = 0;
};

class Interpreter::InitialState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingCommandState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;
};

class Interpreter::AwaitingMoveRoomState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    const State& MovePlayer(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowPathLengthState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    const State& PrepareArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowRoomState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    const State& MoveArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingReplayState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override;
    void OutputStateMessage(Interpreter& interp) const override;

private:
    const State& StartGame(const eventvec& events, Interpreter& interp) const;
};

class Interpreter::EndState : public State
{
public:
    const State& Input(string input, Interpreter& interp) const override { return *this; }
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
    while (m_state != &End && !in.eof())
    {
        strvec output = Input(input);
        for (size_t i = 0; i < output.size(); ++i)
        {
            if (i > 0)
                out << endl;
            out << output[i];
        }
        if (m_state != &End)
            getline(in, input);
    }
}

strvec Interpreter::Input(string input)
{
    m_output.clear();
    m_state = &m_state->Input(input, *this);
    m_state->OutputStateMessage(*this);
    return m_output;
}

const Interpreter::State& Interpreter::InitialState::Input(string input, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);
    interp.Output("");

    if (input == "")
    {
        OutputStateMessage(interp);
        return AwaitingCommand;
    }

    eventvec events = interp.m_commands.RandomPlacements();
    interp.OutputEvents(events);
    return interp.CheckPlayerAlive();
}

void Interpreter::InitialState::OutputStateMessage(Interpreter& interp) const
{
    interp.OutputPlayerState();
    interp.Output("");
}

const Interpreter::State& Interpreter::AwaitingCommandState::Input(string input, Interpreter& interp) const
{
    if (input == "M" || input == "m")
        return AwaitingMoveRoom;
    if (input == "S" || input == "s")
        return AwaitingArrowPathLength;

    if (input != "")
        interp.Output(Msg::Huh);
    return *this;
}

void Interpreter::AwaitingCommandState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::ShootOrMove);
}

const Interpreter::State& Interpreter::AwaitingMoveRoomState::Input(string input, Interpreter& interp) const
{
    if (input == "")
        return *this;

    try
    {
        return MovePlayer(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        return *this;
    }
    catch (const GameException&)
    {
        interp.Output(Msg::Impossible);
        return *this;
    }
}

const Interpreter::State& Interpreter::AwaitingMoveRoomState::MovePlayer(const string& input, Interpreter& interp) const
{
    eventvec events = interp.m_commands.MovePlayer(stoi(input));
    interp.Output("");
    interp.OutputEvents(events);
    return interp.CheckPlayerAlive();
}

void Interpreter::AwaitingMoveRoomState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::WhereTo);
}

const Interpreter::State& Interpreter::AwaitingArrowPathLengthState::Input(string input, Interpreter& interp) const
{
    if (input == "")
        return *this;

    try
    {
        return PrepareArrow(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        return *this;
    }
    catch (const ArrowPathLengthException&)
    {
        interp.Output(Msg::Impossible);
        return *this;
    }
}

const Interpreter::State& Interpreter::AwaitingArrowPathLengthState::PrepareArrow(const string& input, Interpreter& interp) const
{
    interp.m_commands.PrepareArrow(stoi(input));
    return AwaitingArrowRoom;
}

void Interpreter::AwaitingArrowPathLengthState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::NumberOfRooms);
}

const Interpreter::State& Interpreter::AwaitingArrowRoomState::Input(string input, Interpreter& interp) const
{
    if (input == "")
        return *this;

    try
    {
        return MoveArrow(input, interp);
    }
    catch (const exception&)
    {
        interp.Output(Msg::Huh);
        return *this;
    }
    catch (const ArrowDoubleBackException&)
    {
        interp.Output(Msg::NotThatCrooked);
        return *this;
    }
}

const Interpreter::State& Interpreter::AwaitingArrowRoomState::MoveArrow(const string& input, Interpreter& interp) const
{
    eventvec events = interp.m_commands.MoveArrow(stoi(input));
    if (events.empty())
        return *this;

    interp.Output("");
    interp.OutputEvents(events);

    if (!interp.m_playerState.WumpusAlive())
    {
        interp.Output(Msg::GetYouNextTime);
        return End;
    }
    else if (interp.m_playerState.GetArrowsRemaining() == 0)
    {
        interp.Output(Msg::OutOfArrows);
        interp.Output(Msg::YouLose);
        return AwaitingReplay;
    }

    return interp.CheckPlayerAlive();
}

void Interpreter::AwaitingArrowRoomState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::RoomNumber);
}

const Interpreter::State& Interpreter::AwaitingReplayState::Input(string input, Interpreter& interp) const
{
    if (input == "")
        return *this;

    if (input == "Y" || input == "y")
    {
        eventvec events = interp.m_commands.Replay();
        return StartGame(events, interp);
    }
    else if (input == "N" || input == "n")
    {
        eventvec events = interp.m_commands.Restart();
        return StartGame(events, interp);
    }
    else
    {
        interp.Output(Msg::Huh);
        return *this;
    }
}

const Interpreter::State& Interpreter::AwaitingReplayState::StartGame(const eventvec& events, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);
    interp.Output("");
    interp.OutputEvents(events);
    return interp.CheckPlayerAlive();
}

void Interpreter::AwaitingReplayState::OutputStateMessage(Interpreter& interp) const
{
    interp.Output(Msg::SameSetup);
}

const Interpreter::State& Interpreter::CheckPlayerAlive()
{
    if (m_playerState.PlayerAlive())
    {
        OutputPlayerState();
        Output("");
        return AwaitingCommand;
    }
    else
    {
        Output(Msg::YouLose);
        return AwaitingReplay;
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
