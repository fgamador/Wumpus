#include "catch.hpp"

#include <array>

#include "RandomSource.h"

using namespace std;

TEST_CASE("RandomSource")
{
    RandomSource randomSource;
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
        REQUIRE(counts[i] == Approx(333).epsilon(0.1));
    }
}
