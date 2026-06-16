#pragma once
#include <random>
#include <cstdint>

class RNG {
public:
    explicit RNG(unsigned int seed = 0);
    void seed(unsigned int s);

    // [min, max] inclusive
    int range(int min, int max);
    // [0.0, 1.0)
    float frange();
    bool chance(int percent);  // true percent% of the time

private:
    std::mt19937 engine_;
};

extern RNG g_rng;
