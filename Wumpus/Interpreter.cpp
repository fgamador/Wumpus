#include "Interpreter.h"

#include "Msg.h"
#include <sstream>

const string Interpreter::Randomize = "[Randomize]";

const map<Event, string> Interpreter::EventMsgs =
{
    { Event::BatSnatch, Msg::BatSnatch },
    { Event::BumpedWumpus, Msg::BumpedWumpus },
    { Event::EatenByWumpus, Msg::WumpusGotYou },
    { Event::FellInPit, Msg::FellInPit },
    { Event::KilledWumpus, Msg::GotTheWumpus },
    { Event::MissedWumpus, Msg::Missed },
    { Event::ShotSelf, Msg::HitYourself }
};

class Interpreter::State
{
public:
    virtual void OutputEntryMessage(Interpreter& interp) const = 0;

    virtual const State& Input(string input, Interpreter& interp) const
    {
        if (input == "")
            return *this;
        else
            return NonEmptyInput(input, interp);
    }

    virtual const State& NonEmptyInput(string input, Interpreter& interp) const
    {
        return *this;
    }
};

class Interpreter::InitialState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override;
    const State& Input(string input, Interpreter& interp) const override;
};

class Interpreter::AwaitingCommandState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override;
    const State& NonEmptyInput(string input, Interpreter& interp) const override;
};

class Interpreter::AwaitingMoveRoomState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override;
    const State& NonEmptyInput(string input, Interpreter& interp) const override;

private:
    const State& MovePlayer(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowPathLengthState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override;
    const State& NonEmptyInput(string input, Interpreter& interp) const override;

private:
    const State& PrepareArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingArrowRoomState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override;
    const State& NonEmptyInput(string input, Interpreter& interp) const override;

private:
    const State& MoveArrow(const string& input, Interpreter& interp) const;
};

class Interpreter::AwaitingReplayState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override;
    const State& NonEmptyInput(string input, Interpreter& interp) const override;

private:
    const State& StartGame(const eventvec& events, Interpreter& interp) const;
};

class Interpreter::EndState : public State
{
public:
    void OutputEntryMessage(Interpreter& interp) const override {}
    const State& NonEmptyInput(string input, Interpreter& interp) const override { return *this; }
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
    m_state->OutputEntryMessage(*this);
    return m_output;
}

void Interpreter::InitialState::OutputEntryMessage(Interpreter& interp) const
{
}

const Interpreter::State& Interpreter::InitialState::Input(string input, Interpreter& interp) const
{
    interp.Output(Msg::HuntTheWumpus);

    if (input == Randomize)
    {
        eventvec events = interp.m_commands.RandomPlacements();
        return interp.CheckAndOutputPlayerState(events);
    }
    else
    {
        return interp.CheckAndOutputPlayerState({});
    }
}

void Interpreter::AwaitingCommandState::OutputEntryMessage(Interpreter& interp) const
{
    interp.Output(Msg::ShootOrMove);
}

const Interpreter::State& Interpreter::AwaitingCommandState::NonEmptyInput(string input, Interpreter& interp) const
{
    if (input == "M" || input == "m")
    {
        return AwaitingMoveRoom;
    }
    else if (input == "S" || input == "s")
    {
        return AwaitingArrowPathLength;
    }
    else
    {
        interp.Output(Msg::Huh);
        return *this;
    }
}

void Interpreter::AwaitingMoveRoomState::OutputEntryMessage(Interpreter& interp) const
{
    interp.Output(Msg::WhereTo);
}

const Interpreter::State& Interpreter::AwaitingMoveRoomState::NonEmptyInput(string input, Interpreter& interp) const
{
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
    return interp.CheckAndOutputPlayerState(events);
}

void Interpreter::AwaitingArrowPathLengthState::OutputEntryMessage(Interpreter& interp) const
{
    interp.Output(Msg::NumberOfRooms);
}

const Interpreter::State& Interpreter::AwaitingArrowPathLengthState::NonEmptyInput(string input, Interpreter& interp) const
{
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

void Interpreter::AwaitingArrowRoomState::OutputEntryMessage(Interpreter& interp) const
{
    interp.Output(Msg::RoomNumber);
}

const Interpreter::State& Interpreter::AwaitingArrowRoomState::NonEmptyInput(string input, Interpreter& interp) const
{
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

    return interp.CheckAndOutputPlayerState(events);
}

void Interpreter::AwaitingReplayState::OutputEntryMessage(Interpreter& interp) const
{
    interp.Output(Msg::SameSetup);
}

const Interpreter::State& Interpreter::AwaitingReplayState::NonEmptyInput(string input, Interpreter& interp) const
{
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
    return interp.CheckAndOutputPlayerState(events);
}

const Interpreter::State& Interpreter::CheckAndOutputPlayerState(const eventvec& events)
{
    Output("");
    OutputEvents(events);

    if (!m_playerState.WumpusAlive())
        return WumpusDied();
    else if (!m_playerState.PlayerAlive())
        return PlayerDied();
    else if (m_playerState.GetArrowsRemaining() == 0)
        return OutOfArrows();
    else
        return PlayerStillAlive();
}

const Interpreter::State& Interpreter::WumpusDied()
{
    Output(Msg::GetYouNextTime);
    return End;
}

const Interpreter::State& Interpreter::PlayerDied()
{
    Output(Msg::YouLose);
    return AwaitingReplay;
}

const Interpreter::State& Interpreter::OutOfArrows()
{
    Output(Msg::OutOfArrows);
    Output(Msg::YouLose);
    return AwaitingReplay;
}

const Interpreter::State& Interpreter::PlayerStillAlive()
{
    OutputAdjacentHazards();
    OutputPlayerLocation();
    Output("");
    return AwaitingCommand;
}

void Interpreter::OutputEvents(const eventvec& events)
{
    for (Event event : events)
        Output(EventMsgs.at(event));
}

void Interpreter::OutputAdjacentHazards()
{
    if (m_playerState.WumpusAdjacent())
        Output(Msg::SmellWumpus);
    if (m_playerState.BatsAdjacent())
        Output(Msg::BatsNearby);
    if (m_playerState.PitAdjacent())
        Output(Msg::FeelDraft);
}

void Interpreter::OutputPlayerLocation()
{
    Output(Msg::YouAreInRoom + to_string(m_playerState.GetPlayerRoom()));

    ints3 connected = m_playerState.GetPlayerConnectedRooms();
    ostringstream out2;
    out2 << Msg::TunnelsLeadTo << connected[0] << " " << connected[1] << " " << connected[2];
    Output(out2.str());
}

void Interpreter::Output(const string& str)
{
    m_output.push_back(str);
}
