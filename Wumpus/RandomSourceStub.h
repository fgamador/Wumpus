#pragma once

#include "IRandomSource.h"

class RandomSourceStub : public IRandomSource
{
public:
    int NextInt(int from, int to) override
    {
        return m_nextInt;
    }

    void SetNextInt(int val)
    {
        m_nextInt = val;
    }

private:
    int m_nextInt;
};
