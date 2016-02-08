#include "catch.hpp"

#include "CommandInterpreter.h"
#include "Msg.h"
#include "RandomSourceStub.h"

TEST_CASE("In room 2")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);
    CommandInterpreter interpreter(model, model);
    model.SetPlayerRoom(2);

    SECTION("Initial output")
    {
        // TODO need method other than Input("") for this?
        auto output = interpreter.Input("");
        REQUIRE(output == strvec({
            Msg::YouAreInRoom + "2",
            Msg::TunnelsLeadTo + "1 3 10",
            "",
            Msg::ShootOrMove
        }));
    }

    SECTION("Move to room 10")
    {
        interpreter.Input("");
        auto output = interpreter.Input("M");
        REQUIRE(output == strvec({
            Msg::WhereTo
        }));

        output = interpreter.Input("10");
        REQUIRE(output == strvec({
            Msg::YouAreInRoom + "10",
            Msg::TunnelsLeadTo + "2 9 11",
            "",
            Msg::ShootOrMove
        }));
    }
}
