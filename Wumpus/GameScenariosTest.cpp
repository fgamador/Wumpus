#include "catch.hpp"

#include "CommandInterpreter.h"
#include "Msg.h"
#include "RandomSourceStub.h"

TEST_CASE("Full game")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);
    CommandInterpreter interpreter(model, model);

    randomSource.SetNextInts({ 2, 16, 1, 11, 7, 8 });
    model.RandomInit();

    auto output = interpreter.Input("");
    REQUIRE(output == strvec({
        Msg::HuntTheWumpus,
        "",
        Msg::BatsNearby,
        Msg::YouAreInRoom + "2",
        Msg::TunnelsLeadTo + "1 3 10",
        "",
        Msg::ShootOrMove
    }));

    output = interpreter.Input("M");
    REQUIRE(output == strvec({
        Msg::WhereTo
    }));

    randomSource.SetNextInts({ 7 });
    output = interpreter.Input("1");
    REQUIRE(output == strvec({
        Msg::BatSnatch,
        Msg::FellInPit,
        Msg::YouLose,
        Msg::SameSetup
    }));
}
