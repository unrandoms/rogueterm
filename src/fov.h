#pragma once
#include "map.h"

// Shadow-casting FOV — updates Tile::visible flags on the map.
// Marks tiles seen=true when they become visible.
void compute_fov(Map& map, int origin_x, int origin_y, int radius);
