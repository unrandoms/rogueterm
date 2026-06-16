#include "rng.h"
#include <chrono>

RNG g_rng;

RNG::RNG(unsigned int seed) {
    if (seed == 0) {
        seed = static_cast<unsigned int>(
            std::chrono::steady_clock::now().time_since_epoch().count());
    }
    engine_.seed(seed);
}

void RNG::seed(unsigned int s) {
    engine_.seed(s);
}

int RNG::range(int min, int max) {
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(engine_);
}

float RNG::frange() {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(engine_);
}

bool RNG::chance(int percent) {
    return range(1, 100) <= percent;
}
