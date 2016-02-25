#include "catch.hpp"

#include "GameModel.h"
#include "RandomSourceStub.h"

TEST_CASE("GameModel")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);

    SECTION("Random init")
    {
        randomSource.SetNextInts({ 2, 11, 5, 16, 7, 9 });
        model.RandomInit();
        REQUIRE(model.GetPlayerRoom() == 2);
        REQUIRE(model.GetWumpusRoom() == 11);
        REQUIRE(model.GetBatRooms() == ints2({ 5, 16 }));
        REQUIRE(model.GetPitRooms() == ints2({ 7, 9 }));
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
        model.SetWumpusRoom(10);
        model.MovePlayer(10);
        randomSource.SetNextInts({ 9, 3 });

        SECTION("Replay")
        {
            model.Replay();
            REQUIRE(model.PlayerAlive());
            REQUIRE(model.GetPlayerRoom() == 2);
            REQUIRE(model.GetWumpusRoom() == 11);
        }

        SECTION("Restart")
        {
            model.Restart();
            REQUIRE(model.PlayerAlive());
            REQUIRE(model.GetPlayerRoom() == 9);
            REQUIRE(model.GetWumpusRoom() == 3);
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

    SECTION("BatsAdjacent")
    {
        SECTION("Adjacent1")
        {
            model.SetPlayerRoom(2);
            model.SetBatsRooms(10, 19);
            REQUIRE(model.BatsAdjacent());
        }

        SECTION("Adjacent2")
        {
            model.SetPlayerRoom(2);
            model.SetBatsRooms(19, 10);
            REQUIRE(model.BatsAdjacent());
        }

        SECTION("Not adjacent")
        {
            model.SetPlayerRoom(2);
            model.SetBatsRooms(11, 19);
            REQUIRE(!model.BatsAdjacent());
        }
    }

    SECTION("PitAdjacent")
    {
        SECTION("Adjacent1")
        {
            model.SetPlayerRoom(2);
            model.SetPitRooms(10, 19);
            REQUIRE(model.PitAdjacent());
        }

        SECTION("Adjacent2")
        {
            model.SetPlayerRoom(2);
            model.SetPitRooms(19, 10);
            REQUIRE(model.PitAdjacent());
        }

        SECTION("Not adjacent")
        {
            model.SetPlayerRoom(2);
            model.SetPitRooms(11, 19);
            REQUIRE(!model.PitAdjacent());
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

        SECTION("To wumpus room")
        {
            model.SetWumpusRoom(10);

            SECTION("Eaten by wumpus")
            {
                randomSource.SetNextInts({ 3 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::EatenByWumpus
                }));
                REQUIRE(!model.PlayerAlive());
            }

            SECTION("Wumpus moves")
            {
                randomSource.SetNextInts({ 1 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus
                }));
                REQUIRE(model.PlayerAlive());
                REQUIRE(model.GetWumpusRoom() == 9);
            }

            SECTION("No move after death")
            {
                randomSource.SetNextInts({ 3 });
                model.MovePlayer(10);
                REQUIRE_THROWS_AS(model.MovePlayer(2), PlayerDeadException);
            }
        }

        SECTION("To bats room")
        {
            model.SetBatsRooms(10, 19);

            SECTION("One bat snatch")
            {
                randomSource.SetNextInts({ 5 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BatSnatch
                }));
                REQUIRE(model.GetPlayerRoom() == 5);
            }

            SECTION("Two bat snatches")
            {
                randomSource.SetNextInts({ 19, 7 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BatSnatch, Event::BatSnatch
                }));
                REQUIRE(model.GetPlayerRoom() == 7);
            }
        }

        SECTION("To pit room")
        {
            model.SetPitRooms(10, 3);

            SECTION("Fall in pit 1")
            {
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::FellInPit
                }));
                REQUIRE(!model.PlayerAlive());
            }

            SECTION("Fall in pit 2")
            {
                eventvec events = model.MovePlayer(3);
                REQUIRE(events == eventvec({
                    Event::FellInPit
                }));
                REQUIRE(!model.PlayerAlive());
            }
        }
    }

    SECTION("Crooked arrow")
    {
        model.SetPlayerRoom(2);

        SECTION("Invalid path length")
        {
            REQUIRE_THROWS_AS(model.PrepareArrow(0), ArrowPathLengthException);
            REQUIRE_THROWS_AS(model.PrepareArrow(6), ArrowPathLengthException);
        }

        SECTION("Move without Prepare")
        {
            REQUIRE_THROWS_AS(model.MoveArrow(10), ArrowPathLengthException);
        }

        SECTION("Invalid path")
        {
            model.PrepareArrow(2);

            SECTION("No such room")
            {
                REQUIRE_THROWS_AS(model.MoveArrow(0), NoSuchRoomException);
                REQUIRE_THROWS_AS(model.MoveArrow(21), NoSuchRoomException);
            }

            SECTION("Room not connected")
            {
                model.MoveArrow(10);
                REQUIRE_THROWS_AS(model.MoveArrow(5), RoomsNotConnectedException);
            }

            SECTION("Double back")
            {
                model.MoveArrow(10);
                REQUIRE_THROWS_AS(model.MoveArrow(2), ArrowDoubleBackException);
            }

            SECTION("Too many moves")
            {
                model.SetWumpusRoom(1);
                model.MoveArrow(10);
                model.MoveArrow(11);
                REQUIRE_THROWS_AS(model.MoveArrow(12), ArrowPathLengthException);
            }
        }

        SECTION("Kill wumpus on second room of three-room path")
        {
            model.SetWumpusRoom(11);
            model.PrepareArrow(3);
            eventvec events = model.MoveArrow(10);
            REQUIRE(events.empty());
            events = model.MoveArrow(11);
            REQUIRE(events == eventvec({
                Event::KilledWumpus
            }));
            REQUIRE(!model.WumpusAlive());
        }

        SECTION("Killing wumpus ends path")
        {
            model.SetWumpusRoom(10);
            model.PrepareArrow(2);
            model.MoveArrow(10);
            REQUIRE_THROWS_AS(model.MoveArrow(11), ArrowPathLengthException);
        }

        SECTION("Wumpus move after miss")
        {
            randomSource.SetNextInts({ 1 });
            model.SetWumpusRoom(9);
            model.PrepareArrow(2);
            model.MoveArrow(10);
            eventvec events = model.MoveArrow(11);
            REQUIRE(events == eventvec({
                Event::MissedWumpus
            }));
            REQUIRE(model.GetWumpusRoom() == 10);
            REQUIRE(model.WumpusAlive());
        }

        SECTION("Wumpus move to player after miss")
        {
            randomSource.SetNextInts({ 0 });
            model.SetWumpusRoom(1);
            model.PrepareArrow(1);
            eventvec events = model.MoveArrow(10);
            REQUIRE(model.GetWumpusRoom() == 2);
            REQUIRE(model.WumpusAlive());
            REQUIRE(events == eventvec({
                Event::MissedWumpus,
                Event::EatenByWumpus
            }));
            REQUIRE(!model.PlayerAlive());
        }

        SECTION("Hit self")
        {
            model.SetWumpusRoom(20);
            model.PrepareArrow(5);
            model.MoveArrow(3);
            model.MoveArrow(4);
            model.MoveArrow(5);
            model.MoveArrow(1);
            eventvec events = model.MoveArrow(2);
            REQUIRE(events == eventvec({
                Event::ShotSelf
            }));
            REQUIRE(!model.PlayerAlive());
        }
    }
}
