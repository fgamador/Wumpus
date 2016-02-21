#include "catch.hpp"

#include <sstream>

#include "CommandInterpreter.h"
#include "GameCommands.h"
#include "Msg.h"
#include "PlayerState.h"

class GameCommandsSpy : public GameCommands
{
public:
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
        return{ 2, 3, 4 };
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

    bool playerAlive = true;
    bool wumpusAdjacent = false;
    bool batsAdjacent = false;
    bool pitAdjacent = false;
};

namespace {
    void RequireCommands(const GameCommandsSpy& commands, const strvec& invoked)
    {
        REQUIRE(commands.invoked == invoked);
    }

    void RequireOutput(const strvec& output, const strvec& msgs)
    {
        REQUIRE(output == msgs);
    }

    void RequireNextMoveOutput(const strvec& output)
    {
        RequireOutput(output, {
            Msg::YouAreInRoom + "1",
            Msg::TunnelsLeadTo + "2 3 4",
            "",
            Msg::ShootOrMove
        });
    }

    void RequireNextMoveOutput(const strvec& output, const strvec& leadingMsgs)
    {
        for (unsigned i = 0; i < leadingMsgs.size(); ++i)
            REQUIRE(output[i] == leadingMsgs[i]);
        RequireNextMoveOutput(strvec(output.begin() + leadingMsgs.size(), output.end()));
    }
}

TEST_CASE("CommandInterpreter")
{
    GameCommandsSpy commands;
    PlayerStateStub playerState;
    CommandInterpreter interp(commands, playerState);

    SECTION("Initial state")
    {
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output);
    }

    SECTION("Initial state, wumpus adjacent")
    {
        playerState.wumpusAdjacent = true;
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::SmellWumpus });
    }

    SECTION("Initial state, bats adjacent")
    {
        playerState.batsAdjacent = true;
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::BatsNearby });
    }

    SECTION("Initial state, pit adjacent")
    {
        playerState.pitAdjacent = true;
        auto output = interp.Input("");
        RequireCommands(commands, {});
        RequireNextMoveOutput(output, { Msg::FeelDraft });
    }

    SECTION("Awaiting command")
    {
        interp.Input("");

        SECTION("Empty input")
        {
            strvec output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::ShootOrMove });
        }

        SECTION("Unrecognized input")
        {
            strvec output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::ShootOrMove });
        }

        SECTION("M input")
        {
            strvec output = interp.Input("M");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::WhereTo });
        }

        SECTION("m input")
        {
            strvec output = interp.Input("M");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::WhereTo });
        }
    }

    SECTION("Awaiting move room number")
    {
        interp.Input("");
        interp.Input("M");

        SECTION("Empty input")
        {
            strvec output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::WhereTo });
        }

        SECTION("Unparsable input")
        {
            strvec output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::WhereTo });
        }

        SECTION("Room-number input")
        {
            strvec output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireNextMoveOutput(output);
        }

        SECTION("Room-number input, no such room")
        {
            commands.willThrowNoSuchRoomException = true;
            strvec output = interp.Input("21");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Impossible, Msg::WhereTo });
        }

        SECTION("Room-number input, unconnected room")
        {
            commands.willThrowRoomsNotConnectedException = true;
            strvec output = interp.Input("5");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Impossible, Msg::WhereTo });
        }

        SECTION("Bumped wumpus")
        {
            commands.events = { Event::BumpedWumpus };
            strvec output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireNextMoveOutput(output, { Msg::BumpedWumpus });
        }

        SECTION("Bumped and eaten by wumpus")
        {
            commands.events = { Event::BumpedWumpus, Event::EatenByWumpus };
            playerState.playerAlive = false;
            strvec output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireOutput(output, { Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup });
        }

        SECTION("Bat snatch")
        {
            commands.events = { Event::BatSnatch };
            strvec output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireNextMoveOutput(output, { Msg::BatSnatch });
        }

        SECTION("Fell in pit")
        {
            commands.events = { Event::FellInPit };
            playerState.playerAlive = false;
            strvec output = interp.Input("2");
            RequireCommands(commands, { "MovePlayer 2" });
            RequireOutput(output, { Msg::FellInPit, Msg::YouLose, Msg::SameSetup });
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
            strvec output = interp.Input("");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::SameSetup });
        }

        SECTION("Unrecognized input")
        {
            strvec output = interp.Input("X");
            RequireCommands(commands, {});
            RequireOutput(output, { Msg::Huh, Msg::SameSetup });
        }

        SECTION("Y input")
        {
            strvec output = interp.Input("Y");
            RequireCommands(commands, { "Replay" });
            RequireNextMoveOutput(output, { Msg::HuntTheWumpus });
        }

        SECTION("N input")
        {
            strvec output = interp.Input("N");
            RequireCommands(commands, { "Restart" });
            RequireNextMoveOutput(output, { Msg::HuntTheWumpus });
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
            << Msg::YouAreInRoom << "1" << endl
            << Msg::TunnelsLeadTo << "2 3 4" << endl
            << endl
            << Msg::ShootOrMove
            << Msg::WhereTo;
        REQUIRE(out.str() == expected.str());
    }
}
