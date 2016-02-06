#include "catch.hpp"
#include "GameMap.h"
#include <algorithm>
#include <set>

TEST_CASE("GameMap::GetConnectedRooms")
{
    GameMap map;

    SECTION("Room 2")
    {
        REQUIRE(map.GetConnectedRooms(2) == ints3({
            1, 3, 10
        }));
    }

    SECTION("Each room connects to 3 different other rooms")
    {
        for (int room = 1; room <= 20; ++room)
        {
            INFO("Room " << room);
            auto connections = map.GetConnectedRooms(room);
            set<int> rooms(connections.begin(), connections.end());
            rooms.insert(room);
            REQUIRE(rooms.size() == 4);
        }
    }

    SECTION("No room 0")
    {
        REQUIRE_THROWS_AS(map.GetConnectedRooms(0), NoSuchRoomException);
    }

    SECTION("No room 21")
    {
        REQUIRE_THROWS_AS(map.GetConnectedRooms(21), NoSuchRoomException);
    }

    SECTION("Connections are two-way")
    {
        for (int room = 1; room <= 12; ++room)
        {
            auto oneStepAway = map.GetConnectedRooms(room);
            for (int i = 0; i < 3; ++i)
            {
                INFO("Connection " << room << " to " << oneStepAway[i]);
                auto twoStepsAway = map.GetConnectedRooms(oneStepAway[i]);
                auto it = find(twoStepsAway.begin(), twoStepsAway.end(), room);
                REQUIRE(it != twoStepsAway.end());
            }
        }
    }

    SECTION("Connected rooms are connected")
    {
        REQUIRE(map.AreConnected(2, 10));
        REQUIRE(!map.AreConnected(2, 5));
    }
}
