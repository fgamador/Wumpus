#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "Interpreter.h"
#include <iostream>
#include "Model.h"
#include "SimpleRandomSource.h"

int RunTests()
{
    return Catch::Session().run();
}

int RunGame()
{
    SimpleRandomSource randomSource;
    Model model(randomSource);
    Interpreter interp(model, model);

    model.RandomPlacements();
    interp.Run(cin, cout);
    return 0;
}

int main(int argc, const char* argv[])
{
    return (argc > 1) ? RunTests() : RunGame();
}
