#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <iostream>

#include "CommandInterpreter.h"
#include "GameModel.h"
#include "RandomSource.h"

int RunTests()
{
    return Catch::Session().run();
}

int RunGame()
{
    RandomSource randomSource;
    GameModel model(randomSource);
    CommandInterpreter interp(model, model);

    model.RandomPlacements();
    interp.Run(cin, cout);
    return 0;
}

int main(int argc, const char* argv[])
{
    return (argc > 1) ? RunTests() : RunGame();
}
