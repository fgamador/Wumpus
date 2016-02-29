#pragma once

class RandomSource
{
public:
    virtual int NextInt(int from, int to) = 0;
};
