#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <iostream>

#include "CommandInterpreter.h"
#include "GameModel.h"

int RunTests()
{
    return Catch::Session().run();
}

int RunGame()
{
    GameModel model;
    CommandInterpreter interp(model, model);

    // TODO model.RandomInit();
    model.SetPlayerRoom(1);

    interp.Run(cin, cout);

    return 0;
}

int main(int argc, const char* argv[])
{
    return (argc > 1) ? RunTests() : RunGame();
}
