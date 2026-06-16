// Recursive shadow-casting FOV in 8 octants.
// Reference algorithm: Björn Bergström's "Recursive Shadowcasting"
//
// Each octant is transformed into the same coordinate frame (dx always > 0,
// the major axis) via a 2D transform matrix {xx, xy, yx, yy}.
// Within each octant we sweep columns from 1..radius, tracking a stack of
// shadow "slopes" [start_slope, end_slope].

#include "fov.h"
#include <vector>
#include <cmath>

struct Shadow {
    float start, end;
    Shadow(float s, float e) : start(s), end(e) {}
};

// Mark a tile visible (and seen).
static void mark_visible(Map& map, int x, int y) {
    if (map.in_bounds(x, y)) {
        auto& t = map.at(x, y);
        t.visible = true;
        t.seen    = true;
    }
}

// Returns true if slope range [low, high] is fully in shadow list.
static bool is_in_shadow(const std::vector<Shadow>& shadows, float low, float high) {
    for (auto& s : shadows)
        if (s.start <= low && s.end >= high) return true;
    return false;
}

// Merge a new occluder shadow into the list (maintaining sorted, merged order).
static void add_shadow(std::vector<Shadow>& shadows, Shadow proj) {
    size_t idx = 0;
    for (; idx < shadows.size(); ++idx)
        if (shadows[idx].end >= proj.start) break;

    // Merge with existing overlapping shadows
    size_t end_idx = idx;
    while (end_idx < shadows.size() && shadows[end_idx].start <= proj.end) {
        proj.start = std::min(proj.start, shadows[end_idx].start);
        proj.end   = std::max(proj.end,   shadows[end_idx].end);
        ++end_idx;
    }
    shadows.erase(shadows.begin() + idx, shadows.begin() + end_idx);
    shadows.insert(shadows.begin() + idx, proj);
}

// Cast one octant. Transform (col, row) -> world coords via:
//   wx = ox + col*xx + row*xy
//   wy = oy + col*yx + row*yy
static void cast_octant(Map& map, int ox, int oy, int radius,
                         int xx, int xy, int yx, int yy) {
    std::vector<Shadow> shadows;
    bool full_shadow = false;

    for (int col = 1; col <= radius && !full_shadow; ++col) {
        for (int row = 0; row <= col; ++row) {
            int wx = ox + col * xx + row * xy;
            int wy = oy + col * yx + row * yy;

            if (!map.in_bounds(wx, wy)) continue;

            // Slope of this tile's left and right edges
            float l_slope = (row - 0.5f) / (col + 0.5f);
            float r_slope = (row + 0.5f) / (col - 0.5f);

            if (!shadows.empty() && shadows.back().end < l_slope) {
                // Entire remainder of this row is in shadow
                break;
            }

            if (!is_in_shadow(shadows, l_slope, r_slope)) {
                mark_visible(map, wx, wy);

                // If this tile is opaque, add its shadow projection
                if (map.at(wx, wy).type == TileType::Wall) {
                    add_shadow(shadows, Shadow(l_slope, r_slope));

                    // Check if we now cover full shadow
                    if (!shadows.empty() &&
                        shadows.front().start <= -1.0f &&
                        shadows.front().end   >=  1.0f) {
                        full_shadow = true;
                    }
                }
            }
        }
    }
}

// Transform matrices for 8 octants
static const int octant_transforms[8][4] = {
    // xx  xy  yx  yy
    {  1,  0,  0, -1 },  // octant 0: E  (col=dx, row=-dy)
    {  0,  1, -1,  0 },  // octant 1: NE
    {  0, -1, -1,  0 },  // octant 2: NW
    { -1,  0,  0, -1 },  // octant 3: W
    { -1,  0,  0,  1 },  // octant 4: SW
    {  0, -1,  1,  0 },  // octant 5: SE
    {  0,  1,  1,  0 },  // octant 6: SE2
    {  1,  0,  0,  1 },  // octant 7: E2
};

void compute_fov(Map& map, int origin_x, int origin_y, int radius) {
    map.reset_visibility();

    // Origin tile is always visible
    mark_visible(map, origin_x, origin_y);

    for (int i = 0; i < 8; ++i) {
        cast_octant(map, origin_x, origin_y, radius,
                    octant_transforms[i][0],
                    octant_transforms[i][1],
                    octant_transforms[i][2],
                    octant_transforms[i][3]);
    }
}
