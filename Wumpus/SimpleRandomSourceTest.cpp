#include "catch.hpp"

#include "SimpleRandomSource.h"
#include "stdtypes.h"

TEST_CASE("SimpleRandomSource", "[.]")
{
    SimpleRandomSource randomSource;
    array<int, 3> counts = { 0, 0, 0 };
    for (int i = 0; i < 1000; i++)
    {
        int val = randomSource.NextInt(1, 3);
        REQUIRE(val >= 1);
        REQUIRE(val <= 3);
        counts[val-1]++;
    }
    for (int i = 0; i < 3; i++)
    {
        REQUIRE(counts[i] == Approx(333).epsilon(0.2));
    }
}
