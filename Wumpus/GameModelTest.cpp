#include "catch.hpp"

#include "GameModel.h"
#include "RandomSourceStub.h"

TEST_CASE("GameModel")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);

    SECTION("Random init")
    {
        randomSource.SetNextInt(2);
        // TODO stub random somehow
        model.RandomInit();
        REQUIRE(model.GetPlayerRoom() == 2);
    }

    SECTION("SetPlayerRoom")
    {
        SECTION("To valid room")
        {
            model.SetPlayerRoom(2);
            REQUIRE(model.GetPlayerRoom() == 2);
        }

        SECTION("To non-existent room")
        {
            REQUIRE_THROWS_AS(model.SetPlayerRoom(0), NoSuchRoomException);
            REQUIRE_THROWS_AS(model.SetPlayerRoom(21), NoSuchRoomException);
        }
    }

    SECTION("MovePlayer")
    {
        model.SetPlayerRoom(2);

        SECTION("To connected room")
        {
            model.MovePlayer(10);
            REQUIRE(model.GetPlayerRoom() == 10);
        }

        SECTION("To unconnected room")
        {
            REQUIRE_THROWS_AS(model.MovePlayer(5), RoomsNotConnectedException);
        }

        SECTION("To non-existent room")
        {
            REQUIRE_THROWS_AS(model.MovePlayer(0), NoSuchRoomException);
            REQUIRE_THROWS_AS(model.MovePlayer(21), NoSuchRoomException);
        }
    }
}
