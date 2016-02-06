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
            REQUIRE(output == strvec({ Msg::ShootOrMove }));
        }

        SECTION("Unrecognized input")
        {
            strvec output = interpreter.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Huh, Msg::ShootOrMove }));
        }

        SECTION("M input")
        {
            strvec output = interpreter.Input("M");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::WhereTo }));
        }

        SECTION("m input")
        {
            strvec output = interpreter.Input("M");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::WhereTo }));
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
            REQUIRE(output == strvec({ Msg::WhereTo }));
        }

        SECTION("Unparsable input")
        {
            strvec output = interpreter.Input("X");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Huh, Msg::WhereTo }));
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
            REQUIRE(output == strvec({ Msg::Impossible, Msg::WhereTo }));
        }

        SECTION("Room-number input, unconnected room")
        {
            commands.WillThrowRoomsNotConnectedException();
            strvec output = interpreter.Input("5");
            REQUIRE(commands.invoked.empty());
            REQUIRE(output == strvec({ Msg::Impossible, Msg::WhereTo }));
        }
    }

    SECTION("Stream I/O")
    {
        ostringstream input;
        input << "M" << endl;
        ostringstream out;

        interpreter.Run(istringstream(input.str()), out);

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
