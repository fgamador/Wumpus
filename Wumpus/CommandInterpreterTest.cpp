#include "catch.hpp"

#include <sstream>

#include "CommandInterpreter.h"
#include "GameCommands.h"
#include "Msg.h"
#include "PlayerState.h"

namespace {
    void RequireInitialMsg(const strvec& output)
    {
        REQUIRE(output == strvec({
            Msg::YouAreInRoom + "1",
            Msg::TunnelsLeadTo + "2 3 4",
            "",
            Msg::ShootOrMove
        }));
    }
}

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

    bool playerAlive = true;
    bool wumpusAdjacent = false;
    bool batsAdjacent = false;
};

TEST_CASE("CommandInterpreter")
{
    GameCommandsSpy commands;
    PlayerStateStub playerState;
    CommandInterpreter interp(commands, playerState);

    SECTION("Initial state")
    {
        auto output = interp.Input("");
        REQUIRE(commands.invoked.empty());
        RequireInitialMsg(output);
    }

    SECTION("Initial state, wumpus adjacent")
    {
        playerState.wumpusAdjacent = true;
        auto output = interp.Input("");
        REQUIRE(commands.invoked.empty());
        REQUIRE(output[0] == Msg::SmellWumpus);
        RequireInitialMsg(strvec(output.begin() + 1, output.end()));
    }

    SECTION("Initial state, bats adjacent")
    {
        playerState.batsAdjacent = true;
        auto output = interp.Input("");
        REQUIRE(commands.invoked.empty());
        REQUIRE(output[0] == Msg::BatsNearby);
        RequireInitialMsg(strvec(output.begin() + 1, output.end()));
    }

    SECTION("Awaiting command")
    {
        interp.Input("");

        SECTION("Empty input")
        {
            strvec output = interp.Input("");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::ShootOrMove }));
        }

        SECTION("Unrecognized input")
        {
            strvec output = interp.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Huh, Msg::ShootOrMove }));
        }

        SECTION("M input")
        {
            strvec output = interp.Input("M");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::WhereTo }));
        }

        SECTION("m input")
        {
            strvec output = interp.Input("M");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::WhereTo }));
        }
    }

    SECTION("Awaiting move room number")
    {
        interp.Input("");
        interp.Input("M");

        SECTION("Empty input")
        {
            strvec output = interp.Input("");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::WhereTo }));
        }

        SECTION("Unparsable input")
        {
            strvec output = interp.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Huh, Msg::WhereTo }));
        }

        SECTION("Room-number input")
        {
            strvec output = interp.Input("2");
            REQUIRE(commands.invoked == strvec({ "MovePlayer 2" }));
            RequireInitialMsg(output);
        }

        SECTION("Room-number input, no such room")
        {
            commands.willThrowNoSuchRoomException = true;
            strvec output = interp.Input("21");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Impossible, Msg::WhereTo }));
        }

        SECTION("Room-number input, unconnected room")
        {
            commands.willThrowRoomsNotConnectedException = true;
            strvec output = interp.Input("5");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Impossible, Msg::WhereTo }));
        }

        SECTION("Bumped wumpus")
        {
            commands.events = { Event::BumpedWumpus };
            strvec output = interp.Input("2");
            REQUIRE(commands.invoked == strvec({ "MovePlayer 2" }));
            REQUIRE(output[0] == Msg::BumpedWumpus);
            RequireInitialMsg(strvec(output.begin() + 1, output.end()));
        }

        SECTION("Bumped and eaten by wumpus")
        {
            commands.events = { Event::BumpedWumpus, Event::EatenByWumpus };
            playerState.playerAlive = false;
            strvec output = interp.Input("2");
            REQUIRE(commands.invoked == strvec({ "MovePlayer 2" }));
            REQUIRE(output == strvec({ Msg::BumpedWumpus, Msg::WumpusGotYou, Msg::YouLose, Msg::SameSetup }));
        }

        SECTION("Bat snatch")
        {
            commands.events = { Event::BatSnatch };
            strvec output = interp.Input("2");
            REQUIRE(commands.invoked == strvec({ "MovePlayer 2" }));
            REQUIRE(output[0] == Msg::BatSnatch);
            RequireInitialMsg(strvec(output.begin() + 1, output.end()));
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
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::SameSetup }));
        }

        SECTION("Unrecognized input")
        {
            strvec output = interp.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Huh, Msg::SameSetup }));
        }

        SECTION("Y input")
        {
            strvec output = interp.Input("Y");
            REQUIRE(commands.invoked == strvec({ "Replay" }));
            REQUIRE(output[0] == Msg::HuntTheWumpus);
            RequireInitialMsg(strvec(output.begin() + 1, output.end()));
        }

        SECTION("N input")
        {
            strvec output = interp.Input("N");
            REQUIRE(commands.invoked == strvec({ "Restart" }));
            REQUIRE(output[0] == Msg::HuntTheWumpus);
            RequireInitialMsg(strvec(output.begin() + 1, output.end()));
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
