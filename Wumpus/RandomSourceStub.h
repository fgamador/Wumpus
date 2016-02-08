#pragma once

#include "IRandomSource.h"
#include <vector>

using namespace std;

typedef vector<int> intvec;

class RandomSourceStub : public IRandomSource
{
public:
    int NextInt(int from, int to) override
    {
        return *(m_nextInt++);
    }

    void SetNextInts(intvec nextInts)
    {
        m_nextInts = nextInts;
        m_nextInt = m_nextInts.begin();
    }

private:
    intvec m_nextInts;
    intvec::iterator m_nextInt;
};
