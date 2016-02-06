#include "catch.hpp"

#include "GameModel.h"

TEST_CASE("GameModel::SetPlayerRoom")
{
    GameModel model;

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

TEST_CASE("GameModel::MovePlayer")
{
    GameModel model;
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
