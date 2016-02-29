#pragma once

#include <random>
#include "RandomSource.h"

using namespace std;

class SimpleRandomSource : public RandomSource
{
public:
    SimpleRandomSource();

    int NextInt(int from, int to) override;

private:
    minstd_rand0 m_generator;
};
