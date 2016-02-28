#include "catch.hpp"

#include "GameModel.h"
#include "RandomSourceStub.h"

TEST_CASE("Game model")
{
    RandomSourceStub randomSource;
    GameModel model(randomSource);

    SECTION("Random init")
    {
        SECTION("No conflicts")
        {
            randomSource.SetNextInts({ 2, 11, 5, 16, 7, 9 });
            eventvec events = model.RandomInit();
            REQUIRE(events.empty());
            REQUIRE(model.GetPlayerRoom() == 2);
            REQUIRE(model.GetWumpusRoom() == 11);
            REQUIRE(model.GetBatRooms() == ints2({ 5, 16 }));
            REQUIRE(model.GetPitRooms() == ints2({ 7, 9 }));
        }

        SECTION("Start in wumpus room")
        {
            randomSource.SetNextInts({ 2, 2, 5, 16, 7, 9, 3 });
            eventvec events = model.RandomInit();
            REQUIRE(events == eventvec({
                Event::BumpedWumpus, Event::EatenByWumpus
            }));
            REQUIRE(!model.PlayerAlive());
        }
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

        SECTION("Replay")
        {
            randomSource.SetNextInts({ 9, 3 });
            eventvec events = model.Replay();
            REQUIRE(events.empty());
            REQUIRE(model.PlayerAlive());
            REQUIRE(model.GetPlayerRoom() == 2);
            REQUIRE(model.GetWumpusRoom() == 11);
        }

        SECTION("Restart")
        {
            randomSource.SetNextInts({ 9, 3 });
            eventvec events = model.Restart();
            REQUIRE(events.empty());
            REQUIRE(model.PlayerAlive());
            REQUIRE(model.GetPlayerRoom() == 9);
            REQUIRE(model.GetWumpusRoom() == 3);
        }

        SECTION("Restart in wumpus room")
        {
            randomSource.SetNextInts({ 9, 9 });
            eventvec events = model.Restart();
            REQUIRE(events == eventvec({
                Event::BumpedWumpus, Event::EatenByWumpus
            }));
            REQUIRE(!model.PlayerAlive());
        }
    }

    SECTION("Replay after bad start")
    {
        randomSource.SetNextInts({ 2, 2 });
        model.RandomInit();
        randomSource.SetNextInts({ 9, 3 });
        eventvec events = model.Replay();
        REQUIRE(events == eventvec({
            Event::BumpedWumpus, Event::EatenByWumpus
        }));
        REQUIRE(!model.PlayerAlive());
    }

    SECTION("Wumpus adjacent")
    {
        model.SetPlayerRoom(2);

        SECTION("Adjacent")
        {
            model.SetWumpusRoom(10);
            REQUIRE(model.WumpusAdjacent());
        }

        SECTION("Not adjacent")
        {
            model.SetWumpusRoom(11);
            REQUIRE(!model.WumpusAdjacent());
        }
    }

    SECTION("Bats adjacent")
    {
        model.SetPlayerRoom(2);

        SECTION("Adjacent1")
        {
            model.SetBatRooms(10, 19);
            REQUIRE(model.BatsAdjacent());
        }

        SECTION("Adjacent2")
        {
            model.SetBatRooms(19, 10);
            REQUIRE(model.BatsAdjacent());
        }

        SECTION("Not adjacent")
        {
            model.SetBatRooms(11, 19);
            REQUIRE(!model.BatsAdjacent());
        }
    }

    SECTION("Pit adjacent")
    {
        model.SetPlayerRoom(2);

        SECTION("Adjacent1")
        {
            model.SetPitRooms(10, 19);
            REQUIRE(model.PitAdjacent());
        }

        SECTION("Adjacent2")
        {
            model.SetPitRooms(19, 10);
            REQUIRE(model.PitAdjacent());
        }

        SECTION("Not adjacent")
        {
            model.SetPitRooms(11, 19);
            REQUIRE(!model.PitAdjacent());
        }
    }

    SECTION("Move player")
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

        SECTION("To bat room")
        {
            model.SetBatRooms(10, 19);

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

        SECTION("To wumpus and bat room")
        {
            model.SetWumpusRoom(10);
            model.SetBatRooms(10, 19);

            SECTION("Bat snatch, wumpus stays put")
            {
                randomSource.SetNextInts({ 5, 3 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::BatSnatch
                }));
                REQUIRE(model.PlayerAlive());
                REQUIRE(model.GetPlayerRoom() == 5);
                REQUIRE(model.GetWumpusRoom() == 10);
            }

            SECTION("Bat snatch, wumpus moves elsewhere")
            {
                randomSource.SetNextInts({ 5, 1 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::BatSnatch
                }));
                REQUIRE(model.PlayerAlive());
                REQUIRE(model.GetPlayerRoom() == 5);
                REQUIRE(model.GetWumpusRoom() == 9);
            }

            SECTION("Bat snatch, wumpus moves to player room")
            {
                randomSource.SetNextInts({ 9, 1 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::BatSnatch, Event::EatenByWumpus
                }));
                REQUIRE(!model.PlayerAlive());
                REQUIRE(model.GetPlayerRoom() == 9);
                REQUIRE(model.GetWumpusRoom() == 9);
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

        SECTION("To wumpus and pit room")
        {
            model.SetWumpusRoom(10);
            model.SetPitRooms(10, 3);

            SECTION("Wumpus stays put")
            {
                randomSource.SetNextInts({ 3 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::EatenByWumpus
                }));
                REQUIRE(!model.PlayerAlive());
            }

            SECTION("Wumpus moves elsewhere")
            {
                randomSource.SetNextInts({ 1 });
                eventvec events = model.MovePlayer(10);
                REQUIRE(events == eventvec({
                    Event::BumpedWumpus, Event::FellInPit
                }));
                REQUIRE(!model.PlayerAlive());
            }
        }

        SECTION("To bat and pit room")
        {
            model.SetBatRooms(10, 19);
            model.SetPitRooms(10, 19);
            randomSource.SetNextInts({ 5 });
            eventvec events = model.MovePlayer(10);
            REQUIRE(events == eventvec({
                Event::BatSnatch
            }));
            REQUIRE(model.GetPlayerRoom() == 5);
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

        SECTION("Arrow already prepared")
        {
            model.PrepareArrow(1);
            REQUIRE_THROWS_AS(model.PrepareArrow(1), ArrowAlreadyPreparedException);
        }

        SECTION("Out of arrows")
        {
            model.SetWumpusRoom(20);
            for (int i = 0; i < GameModel::MaxArrows; ++i)
            {
                model.PrepareArrow(1);
                model.MoveArrow(10);
            }
            REQUIRE_THROWS_AS(model.PrepareArrow(1), OutOfArrowsException);
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
