#include "catch.hpp"

#include "GameModel.h"
#include "RandomSourceStub.h"

TEST_CASE("GameModel")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);

    SECTION("Random init")
    {
        randomSource.SetNextInts({ 2, 11 });
        model.RandomInit();
        REQUIRE(model.GetPlayerRoom() == 2);
        REQUIRE(model.GetWumpusRoom() == 11);
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

    SECTION("WumpusAdjacent")
    {
        SECTION("Adjacent")
        {
            model.SetPlayerRoom(2);
            model.SetWumpusRoom(10);
            REQUIRE(model.WumpusAdjacent());
        }

        SECTION("Not adjacent")
        {
            model.SetPlayerRoom(2);
            model.SetWumpusRoom(11);
            REQUIRE(!model.WumpusAdjacent());
        }
    }

    SECTION("MovePlayer")
    {
        model.SetPlayerRoom(2);

        SECTION("To connected room")
        {
            set<Event> events = model.MovePlayer(10);
            REQUIRE(events.empty());
            REQUIRE(model.GetPlayerRoom() == 10);
            REQUIRE(model.PlayerAlive());
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

        SECTION("To Wumpus room")
        {
            model.SetWumpusRoom(10);
            set<Event> events = model.MovePlayer(10);
            REQUIRE(events.size() == 1);
            REQUIRE(events.count(Event::EatenByWumpus) == 1);
            REQUIRE(!model.PlayerAlive());
        }
    }
}
