#include "catch.hpp"

#include "CommandInterpreter.h"

TEST_CASE("In room 2")
{
    GameModel model;
    CommandInterpreter interpreter(model, model);
    model.SetPlayerRoom(2);

    SECTION("Initial output")
    {
        // TODO need method other than Input("") for this?
        auto output = interpreter.Input("");
        REQUIRE(output == strvec({
            "YOU ARE IN ROOM 2",
            "TUNNELS LEAD TO 1 3 10",
            "",
            "SHOOT OR MOVE (S-M)? "
        }));
    }

    SECTION("Move to room 10")
    {
        interpreter.Input("");
        auto output = interpreter.Input("M");
        REQUIRE(output == strvec({
            "WHERE TO? "
        }));

        output = interpreter.Input("10");
        REQUIRE(output == strvec({
            "YOU ARE IN ROOM 10",
            "TUNNELS LEAD TO 2 9 11",
            "",
            "SHOOT OR MOVE (S-M)? "
        }));
    }
}
