#include "catch.hpp"

#include <sstream>

#include "Interpreter.h"
#include "Commands.h"
#include "Msg.h"
#include "PlayerState.h"

class CommandsSpy : public Commands
{
public:
    eventvec RandomPlacements() override
    {
        invoked.push_back("RandomPlacements");
        return PostClearEvents();
    }

    eventvec MovePlayer(int room) override
    {
        if (willThrowNoSuchRoomException)
            throw NoSuchRoomException();
        if (willThrowPlayerDeadException)
            throw PlayerDeadException();
        if (willThrowRoomsNotConnectedException)
            throw RoomsNotConnectedException();

        invoked.push_back("MovePlayer " + to_string(room));
        return PostClearEvents();
    }

    void PrepareArrow(int numRooms) override
    {
        if (willThrowArrowPathLengthException)
            throw ArrowPathLengthException();

        invoked.push_back("PrepareArrow " + to_string(numRooms));
        events.clear();
    }

    eventvec MoveArrow(int room) override
    {
        if (willThrowArrowDoubleBackException)
            throw ArrowDoubleBackException();
        if (willThrowArrowPathLengthException)
            throw ArrowPathLengthException();

        invoked.push_back("MoveArrow " + to_string(room));
        return PostClearEvents();
    }

    eventvec Replay() override
    {
        invoked.push_back("Replay");
        return PostClearEvents();
    }

    eventvec Restart() override
    {
        invoked.push_back("Restart");
        return PostClearEvents();
    }

    eventvec PostClearEvents()
    {
        eventvec retval = events;
        events.clear();
        return retval;
    }

    strvec invoked;
    bool willThrowNoSuchRoomException = false;
    bool willThrowPlayerDeadException = false;
    bool willThrowRoomsNotConnectedException = false;
    bool willThrowArrowPathLengthException = false;
    bool willThrowArrowDoubleBackException = false;
    eventvec events = {};
};

class PlayerStateStub : public PlayerState
{
public:
    bool PlayerAlive() const
    {
        return playerAlive;
    }

    int GetPlayerRoom() const
    {
        return 1;
    }

    ints3 GetPlayerConnectedRooms() const
    {
        return { 2, 3, 4 };
    }

    bool WumpusAdjacent() const
    {
        return wumpusAdjacent;
    }

    bool BatsAdjacent() const
    {
        return batsAdjacent;
    }

    bool PitAdjacent() const
    {
        return pitAdjacent;
    }

    bool WumpusAlive() const
    {
        return wumpusAlive;
    }

    int GetArrowsRemaining() const
    {
        return arrowsRemaining;
    }

    bool playerAlive = true;
    bool wumpusAdjacent = false;
    bool batsAdjacent = false;
    bool pitAdjacent = false;
    bool wumpusAlive = true;
    int arrowsRemaining = 5;
};

namespace {
    void RequireCommands(const CommandsSpy& commands, const strvec& invoked)
    {
        REQUIRE(commands.invoked == invoked);
    }

    void RequireOutput(const strvec& output, const strvec& msgs)
    {
        REQUIRE(output == msgs);
    }

    void RequireOutput(const strvec& output, const strvec& msgs, unsigned& outputIndex)
    {
        for (unsigned i = 0; i < msgs.size(); ++i)
            REQUIRE(output[outputIndex++] == msgs[i]);
    }

    void RequireNextMoveOutput(const strvec& output, const strvec& leadingMsgs)
    {
        REQUIRE(output.size() >= leadingMsgs.size());
        unsigned outputIndex = 0;
        RequireOutput(output, leadingMsgs, outputIndex);
        RequireOutput(output, {
            Msg::YouAreInRoom + "1",
            Msg::TunnelsLeadTo + "2 3 4",
            "",
            Msg::ShootOrMove
        }, outputIndex);
    }
}

TEST_CASE("Interpreter")
{
    CommandsSpy commands;
    PlayerStateStub playerState;
    Interpreter interp(commands, playerState);

    SECTION("Initial state")
    {
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "" });
    }

    SECTION("Initial state, wumpus adjacent")
    {
        playerState.wumpusAdjacent = true;
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "", Msg::SmellWumpus });
    }

    SECTION("Initial state, bats adjacent")
    {
        playerState.batsAdjacent = true;
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "", Msg::BatsNearby });
    }

    SECTION("Initial state, pit adjacent")
    {
        playerState.pitAdjacent = true;
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "", Msg::FeelDraft });
    }

    SECTION("Initial state, random init")
    {
        auto output = interp.Input(Interpreter::Randomize);
        RequireCommands(commands, { "RandomPlacements" });
        RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "" });
    }

    SECTION("Initial state, random init, in wumpus room")
    {
        commands.events = { Event::BumpedWumpus, Event::EatenByWumpus };
        playerState.playerAlive = false;
        auto output = interp.Input(Interpreter::Randomize);
        RequireCommands(commands, { "RandomPlacements" });
        RequireOutput(output, { Msg::HuntTheWumpus, "", Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup });
    }

    SECTION("Awaiting command")
    {
        interp.Input("");

        SECTION("Empty input")
        {
            auto output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::ShootOrMove });
        }

        SECTION("Unrecognized input")
        {
            auto output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::ShootOrMove });
        }

        SECTION("M input")
        {
            auto output = interp.Input("M");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::WhereTo });
        }

        SECTION("S input")
        {
            auto output = interp.Input("S");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::NumberOfRooms });
        }
    }

    SECTION("Awaiting move room number")
    {
        interp.Input("");
        interp.Input("M");

        SECTION("Empty input")
        {
            auto output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::WhereTo });
        }

        SECTION("Unparsable input")
        {
            auto output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::WhereTo });
        }

        SECTION("Valid input")
        {
            auto output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireNextMoveOutput(output, { "" });
        }

        SECTION("Room-number input, no such room")
        {
            commands.willThrowNoSuchRoomException = true;
            auto output = interp.Input("21");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Impossible, Msg::WhereTo });
        }

        SECTION("Room-number input, unconnected room")
        {
            commands.willThrowRoomsNotConnectedException = true;
            auto output = interp.Input("5");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Impossible, Msg::WhereTo });
        }

        SECTION("Bumped wumpus")
        {
            commands.events = { Event::BumpedWumpus };
            auto output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireNextMoveOutput(output, { "", Msg::BumpedWumpus });
        }

        SECTION("Bumped and eaten by wumpus")
        {
            commands.events = { Event::BumpedWumpus, Event::EatenByWumpus };
            playerState.playerAlive = false;
            auto output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireOutput(output, { "", Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup });
        }

        SECTION("Bat snatch")
        {
            commands.events = { Event::BatSnatch };
            auto output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireNextMoveOutput(output, { "", Msg::BatSnatch });
        }

        SECTION("Fell in pit")
        {
            commands.events = { Event::FellInPit };
            playerState.playerAlive = false;
            auto output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireOutput(output, { "", Msg::FellInPit, Msg::YouLose, Msg::SameSetup });
        }
    }

    SECTION("Awaiting arrow path length")
    {
        interp.Input("");
        interp.Input("S");

        SECTION("Empty input")
        {
            auto output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::NumberOfRooms });
        }

        SECTION("Unparsable input")
        {
            auto output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::NumberOfRooms });
        }

        SECTION("Valid input")
        {
            auto output = interp.Input("2");
            RequireCommands(commands, { "PrepareArrow 2" });
            RequireOutput(output, { Msg::RoomNumber });
        }

        SECTION("Invalid length")
        {
            commands.willThrowArrowPathLengthException = true;
            auto output = interp.Input("6");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Impossible, Msg::NumberOfRooms });
        }
    }

    SECTION("Awaiting arrow room")
    {
        interp.Input("");
        interp.Input("S");
        interp.Input("3");
        commands.invoked.clear();

        SECTION("Empty input")
        {
            auto output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::RoomNumber });
        }

        SECTION("Unparsable input")
        {
            auto output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::RoomNumber });
        }

        SECTION("Double back")
        {
            commands.willThrowArrowDoubleBackException = true;
            auto output = interp.Input("6");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::NotThatCrooked, Msg::RoomNumber });
        }

        SECTION("Valid input")
        {
            auto output = interp.Input("10");
            RequireCommands(commands, { "MoveArrow 10" });
            RequireOutput(output, { Msg::RoomNumber });
        }

        SECTION("Full path, miss")
        {
            interp.Input("10");
            interp.Input("11");
            commands.events = { Event::MissedWumpus };
            auto output = interp.Input("12");
            RequireCommands(commands, { "MoveArrow 10", "MoveArrow 11", "MoveArrow 12" });
            RequireNextMoveOutput(output, { "", Msg::Missed });
        }

        SECTION("Miss, wumpus move to player")
        {
            commands.events = { Event::MissedWumpus, Event::BumpedWumpus, Event::EatenByWumpus };
            playerState.playerAlive = false;
            auto output = interp.Input("10");
            RequireCommands(commands, { "MoveArrow 10" });
            RequireOutput(output, { "", Msg::Missed, Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup });
        }

        SECTION("Miss with last arrow")
        {
            commands.events = { Event::MissedWumpus };
            playerState.arrowsRemaining = 0;
            auto output = interp.Input("10");
            RequireCommands(commands, { "MoveArrow 10" });
            RequireOutput(output, { "", Msg::Missed, Msg::OutOfArrows, Msg::YouLose, Msg::SameSetup });
        }

        SECTION("Full path, hit")
        {
            interp.Input("10");
            interp.Input("11");
            commands.events = { Event::KilledWumpus };
            playerState.wumpusAlive = false;
            auto output = interp.Input("12");
            RequireCommands(commands, { "MoveArrow 10", "MoveArrow 11", "MoveArrow 12" });
            RequireOutput(output, { "", Msg::GotTheWumpus, Msg::GetYouNextTime });
        }
    }

    SECTION("Awaiting replay")
    {
        interp.Input("");
        interp.Input("M");
        playerState.playerAlive = false;
        interp.Input("2");
        commands.invoked.clear();

        SECTION("Empty input")
        {
            auto output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::SameSetup });
        }

        SECTION("Unrecognized input")
        {
            auto output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::SameSetup });
        }

        SECTION("Y input")
        {
            playerState.playerAlive = true;
            auto output = interp.Input("Y");
            RequireCommands(commands, { "Replay" });
            RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "" });
        }

        SECTION("Y input, restart in wumpus room")
        {
            commands.events = { Event::BumpedWumpus, Event::EatenByWumpus };
            auto output = interp.Input("Y");
            RequireCommands(commands, { "Replay" });
            RequireOutput(output, { Msg::HuntTheWumpus, "", Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup });
        }

        SECTION("N input")
        {
            playerState.playerAlive = true;
            auto output = interp.Input("N");
            RequireCommands(commands, { "Restart" });
            RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "" });
        }

        SECTION("N input, restart in wumpus room")
        {
            commands.events = { Event::BumpedWumpus, Event::EatenByWumpus };
            auto output = interp.Input("N");
            RequireCommands(commands, { "Restart" });
            RequireOutput(output, { Msg::HuntTheWumpus, "", Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup });
        }
    }

    SECTION("Stream I/O")
    {
        stringstream in;
        in << "M" << endl;
        ostringstream out;

        interp.Run(in, out);

        ostringstream expected;
        expected
            << Msg::HuntTheWumpus << endl
            << endl
            << Msg::YouAreInRoom << "1" << endl
            << Msg::TunnelsLeadTo << "2 3 4" << endl
            << endl
            << Msg::ShootOrMove
            << Msg::WhereTo;
        REQUIRE(out.str() == expected.str());
    }
}
