#pragma once

class IRandomSource
{
public:
    virtual int NextInt(int from, int to) = 0;
};
