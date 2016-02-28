#include "catch.hpp"

#include "CommandInterpreter.h"
#include "Msg.h"
#include "RandomSourceStub.h"

namespace {
    void RequireOutput(const strvec& output, const strvec& msgs)
    {
        REQUIRE(output == msgs);
    }

    void RequireNextMoveOutput(const strvec& output, int playerRoom, intvec connectedRooms)
    {
        RequireOutput(output, {
            Msg::YouAreInRoom + to_string(playerRoom),
            Msg::TunnelsLeadTo + to_string(connectedRooms[0]) + " " + to_string(connectedRooms[1]) + " " + to_string(connectedRooms[2]),
            "",
            Msg::ShootOrMove
        });
    }

    void RequireNextMoveOutput(const strvec& output, const strvec& leadingMsgs, int playerRoom, intvec connectedRooms)
    {
        for (unsigned i = 0; i < leadingMsgs.size(); ++i)
            REQUIRE(output[i] == leadingMsgs[i]);
        RequireNextMoveOutput(strvec(output.begin() + leadingMsgs.size(), output.end()), playerRoom, connectedRooms);
    }
}

TEST_CASE("Article full game")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);
    CommandInterpreter interp(model, model);

    randomSource.SetNextInts({ 2, 16, 1, 11, 7, 8 });
    model.RandomPlacements();

    auto output = interp.Input("");
    RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "", Msg::BatsNearby }, 2, { 1, 3, 10 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    randomSource.SetNextInts({ 7 });
    output = interp.Input("1");
    RequireOutput(output, { Msg::BatSnatch, Msg::FellInPit, Msg::YouLose, Msg::SameSetup });

    output = interp.Input("Y");
    RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "", Msg::BatsNearby }, 2, { 1, 3, 10 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("3");
    RequireNextMoveOutput(output, 3, { 2, 4, 12 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("4");
    RequireNextMoveOutput(output, 4, { 3, 5, 14 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("5");
    RequireNextMoveOutput(output, { Msg::BatsNearby }, 5, { 1, 4, 6 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("6");
    RequireNextMoveOutput(output, { Msg::FeelDraft }, 6, { 5, 7, 15 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("7");
    RequireOutput(output, { Msg::FellInPit, Msg::YouLose, Msg::SameSetup });

    output = interp.Input("Y");
    RequireNextMoveOutput(output, { Msg::HuntTheWumpus, "", Msg::BatsNearby }, 2, { 1, 3, 10 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("10");
    RequireNextMoveOutput(output, { Msg::BatsNearby }, 10, { 2, 9, 11 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    randomSource.SetNextInts({ 14 });
    output = interp.Input("11");
    RequireNextMoveOutput(output, { Msg::BatSnatch }, 14, { 4, 13, 15 });

    output = interp.Input("M");
    RequireOutput(output, { Msg::WhereTo });

    output = interp.Input("15");
    RequireNextMoveOutput(output, { Msg::SmellWumpus }, 15, { 6, 14, 16 });

    output = interp.Input("S");
    RequireOutput(output, { Msg::NumberOfRooms });

    output = interp.Input("1");
    RequireOutput(output, { Msg::RoomNumber });

    output = interp.Input("16");
    RequireOutput(output, { Msg::GotTheWumpus, Msg::GetYouNextTime });
}
