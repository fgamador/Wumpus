#include "RandomSource.h"

#include <chrono>

using namespace chrono;

RandomSource::RandomSource()
    : m_generator(static_cast<unsigned int>(system_clock::now().time_since_epoch().count()))
{
}

int RandomSource::NextInt(int from, int to)
{
    uniform_int_distribution<int> distribution(from, to);
    return distribution(m_generator);
}
