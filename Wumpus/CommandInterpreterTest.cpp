#include "catch.hpp"

#include "CommandInterpreter.h"
#include "GameCommands.h"
#include "PlayerState.h"

namespace {
    const string Impossible = "IMPOSSIBLE";
    const string ShootOrMove = "SHOOT OR MOVE (S-M)? ";
    const string UnrecognizedInput = "HUH?";
    const string WhereTo = "WHERE TO? ";

    void RequireInitialMsg(const strvec& output)
    {
        REQUIRE(output == strvec({
            "YOU ARE IN ROOM 1",
            "TUNNELS LEAD TO 2 3 4",
            "",
            ShootOrMove
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
};

TEST_CASE("CommandInterpreter")
{
    GameCommandsSpy commands;
    CommandInterpreter interpreter(commands, PlayerStateStub());

    SECTION("Initial state")
    {
        auto output = interpreter.Input("");
        REQUIRE(commands.invoked.empty());
        RequireInitialMsg(output);
    }

    SECTION("Awaiting command")
    {
        interpreter.Input("");

        SECTION("Empty input")
        {
            strvec output = interpreter.Input("");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ ShootOrMove }));
        }

        SECTION("Unrecognized input")
        {
            strvec output = interpreter.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ UnrecognizedInput, ShootOrMove }));
        }

        SECTION("M input")
        {
            strvec output = interpreter.Input("M");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ WhereTo }));
        }

        SECTION("m input")
        {
            strvec output = interpreter.Input("M");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ WhereTo }));
        }
    }

    SECTION("Awaiting move room number")
    {
        interpreter.Input("");
        interpreter.Input("M");

        SECTION("Empty input")
        {
            strvec output = interpreter.Input("");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ WhereTo }));
        }

        SECTION("Unparsable input")
        {
            strvec output = interpreter.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ UnrecognizedInput, WhereTo }));
        }

        SECTION("Room-number input")
        {
            strvec output = interpreter.Input("2");
            REQUIRE(commands.invoked == strvec({ "MovePlayer 2" }));
            RequireInitialMsg(output);
        }

        SECTION("Room-number input, no such room")
        {
            commands.WillThrowNoSuchRoomException();
            strvec output = interpreter.Input("21");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Impossible, WhereTo }));
        }

        SECTION("Room-number input, unconnected room")
        {
            commands.WillThrowRoomsNotConnectedException();
            strvec output = interpreter.Input("5");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Impossible, WhereTo }));
        }
    }
}
