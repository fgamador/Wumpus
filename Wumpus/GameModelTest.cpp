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

    SECTION("Start over")
    {
        randomSource.SetNextInts({ 2, 11 });
        model.RandomInit();
        randomSource.SetNextInts({ 9, 3 });

        SECTION("Replay")
        {
            model.MovePlayer(10);
            //model.MoveWumpus(19);
            model.Replay();
            REQUIRE(model.GetPlayerRoom() == 2);
            //REQUIRE(model.GetWumpusRoom() == 11);
        }

        SECTION("Restart")
        {
            model.MovePlayer(10);
            //model.MoveWumpus(19);
            model.Restart();
            REQUIRE(model.GetPlayerRoom() == 9);
            //REQUIRE(model.GetWumpusRoom() == 3);
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
            eventvec events = model.MovePlayer(10);
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

            SECTION("Eaten by Wumpus")
            {
                randomSource.SetNextInts({ 1 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::EatenByWumpus
                }));
                REQUIRE(!model.PlayerAlive());
            }

            SECTION("Wumpus moves")
            {
                randomSource.SetNextInts({ 4 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus
                }));
                REQUIRE(model.PlayerAlive());
            }

            SECTION("No move after death")
            {
                randomSource.SetNextInts({ 1 });
                model.MovePlayer(10);
                REQUIRE_THROWS_AS(model.MovePlayer(2), PlayerDeadException);
            }
        }
    }
}
