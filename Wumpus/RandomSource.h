#pragma once

#include <random>
#include "IRandomSource.h"

using namespace std;

class RandomSource : public IRandomSource
{
public:
    RandomSource();

    int NextInt(int from, int to) override;

private:
    minstd_rand0 m_generator;
};
