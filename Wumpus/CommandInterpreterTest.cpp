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
    void WillThrowNoSuchRoomException()
    {
        m_willThrowNoSuchRoomException = true;
    }

    void WillThrowRoomsNotConnectedException()
    {
        m_willThrowRoomsNotConnectedException = true;
    }

    void MovePlayer(int room) override
    {
        if (m_willThrowNoSuchRoomException)
            throw NoSuchRoomException();
        if (m_willThrowRoomsNotConnectedException)
            throw RoomsNotConnectedException();

        invoked.push_back("MovePlayer " + to_string(room));
    }

    strvec invoked;

private:
    bool m_willThrowNoSuchRoomException = false;
    bool m_willThrowRoomsNotConnectedException = false;
};

class PlayerStateStub : public PlayerState
{
public:
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

    bool wumpusAdjacent = false;
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
            commands.WillThrowNoSuchRoomException();
            strvec output = interp.Input("21");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Impossible, Msg::WhereTo }));
        }

        SECTION("Room-number input, unconnected room")
        {
            commands.WillThrowRoomsNotConnectedException();
            strvec output = interp.Input("5");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Impossible, Msg::WhereTo }));
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
