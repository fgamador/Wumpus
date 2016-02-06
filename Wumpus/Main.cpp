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

    string input;
    for (;;)
    {
        strvec output = interp.Input(input);
        for (size_t i = 0; i < output.size(); ++i)
        {
            if (i > 0)
                cout << endl;
            cout << output[i];
        }
        getline(cin, input);
    }

    return 0;
}

int main(int argc, const char* argv[])
{
    return (argc > 1) ? RunTests() : RunGame();
}
