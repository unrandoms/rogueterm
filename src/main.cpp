#include "game.h"

int main(int argc, char* argv[]) {
    unsigned int seed = 0;
    if (argc > 1) {
        try { seed = static_cast<unsigned int>(std::stoul(argv[1])); }
        catch (...) { seed = 0; }
    }
    Game game(seed);
    game.run();
    return 0;
}
