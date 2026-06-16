#include "map.h"
#include "rng.h"
#include <algorithm>
#include <cmath>

Map::Map() : tiles_(MAP_W * MAP_H) {}

void Map::generate(int floor_num) {
    // Reset all tiles to walls
    for (auto& t : tiles_) {
        t.type    = TileType::Wall;
        t.visible = false;
        t.seen    = false;
    }
    rooms_.clear();

    // BSP tree leaf collection
    std::vector<Rect> leaves;
    bsp_split(1, 1, MAP_W - 2, MAP_H - 2, 0, leaves);

    // Carve rooms inside each leaf
    for (auto& leaf : leaves) {
        // Random room size within the leaf, at least 4x4
        int min_w = std::min(4, leaf.w - 2);
        int min_h = std::min(4, leaf.h - 2);
        int rw = (leaf.w <= min_w + 2) ? leaf.w
                                       : g_rng.range(min_w, leaf.w - 1);
        int rh = (leaf.h <= min_h + 2) ? leaf.h
                                       : g_rng.range(min_h, leaf.h - 1);
        rw = std::max(rw, 4);
        rh = std::max(rh, 4);
        int rx = leaf.x + g_rng.range(0, leaf.w - rw - 1);
        int ry = leaf.y + g_rng.range(0, leaf.h - rh - 1);
        rx = std::clamp(rx, 1, MAP_W - rw - 2);
        ry = std::clamp(ry, 1, MAP_H - rh - 2);
        Rect room{rx, ry, rw, rh};
        carve_room(room);
        rooms_.push_back(room);
    }

    // Connect rooms with L-shaped corridors between consecutive rooms
    for (size_t i = 1; i < rooms_.size(); ++i) {
        carve_corridor(rooms_[i-1].cx(), rooms_[i-1].cy(),
                       rooms_[i].cx(),   rooms_[i].cy());
    }

    // Player starts in first room
    player_start_x_ = rooms_.front().cx();
    player_start_y_ = rooms_.front().cy();

    // Stairs in last room
    place_stairs();

    // Silence unused floor_num warning — higher floors could be harder (future)
    (void)floor_num;
}

void Map::bsp_split(int x, int y, int w, int h, int depth,
                    std::vector<Rect>& leaves) {
    constexpr int MIN_SIZE   = 10;
    constexpr int MAX_DEPTH  = 4;   // gives 8-16 leaf rooms

    bool can_split_h = (h >= MIN_SIZE * 2);
    bool can_split_v = (w >= MIN_SIZE * 2);

    if (depth >= MAX_DEPTH || (!can_split_h && !can_split_v)) {
        leaves.push_back({x, y, w, h});
        return;
    }

    bool split_horizontal;
    if (!can_split_h)      split_horizontal = false;
    else if (!can_split_v) split_horizontal = true;
    else                   split_horizontal = g_rng.chance(50);

    if (split_horizontal) {
        int split = g_rng.range(MIN_SIZE, h - MIN_SIZE);
        bsp_split(x, y,          w, split,     depth + 1, leaves);
        bsp_split(x, y + split,  w, h - split, depth + 1, leaves);
    } else {
        int split = g_rng.range(MIN_SIZE, w - MIN_SIZE);
        bsp_split(x,          y, split,     h, depth + 1, leaves);
        bsp_split(x + split,  y, w - split, h, depth + 1, leaves);
    }
}

void Map::carve_room(const Rect& r) {
    for (int dy = 0; dy < r.h; ++dy) {
        for (int dx = 0; dx < r.w; ++dx) {
            int tx = r.x + dx;
            int ty = r.y + dy;
            if (in_bounds(tx, ty))
                at(tx, ty).type = TileType::Floor;
        }
    }
}

void Map::carve_corridor(int x1, int y1, int x2, int y2) {
    // L-shaped: horizontal then vertical (random choice which goes first)
    if (g_rng.chance(50)) {
        // Horizontal first
        int step = (x2 > x1) ? 1 : -1;
        for (int x = x1; x != x2; x += step)
            if (in_bounds(x, y1)) at(x, y1).type = TileType::Floor;
        step = (y2 > y1) ? 1 : -1;
        for (int y = y1; y != y2 + step; y += step)
            if (in_bounds(x2, y)) at(x2, y).type = TileType::Floor;
    } else {
        // Vertical first
        int step = (y2 > y1) ? 1 : -1;
        for (int y = y1; y != y2; y += step)
            if (in_bounds(x1, y)) at(x1, y).type = TileType::Floor;
        step = (x2 > x1) ? 1 : -1;
        for (int x = x1; x != x2 + step; x += step)
            if (in_bounds(x, y2)) at(x, y2).type = TileType::Floor;
    }
}

void Map::place_stairs() {
    // Try to place stairs in last room center; fallback to any floor tile
    if (!rooms_.empty()) {
        stairs_x_ = rooms_.back().cx();
        stairs_y_ = rooms_.back().cy();
        at(stairs_x_, stairs_y_).type = TileType::Stairs;
        return;
    }
    // Fallback
    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x)
            if (at(x, y).type == TileType::Floor) {
                stairs_x_ = x; stairs_y_ = y;
                at(x, y).type = TileType::Stairs;
                return;
            }
}

void Map::reset_visibility() {
    for (auto& t : tiles_) t.visible = false;
}

bool Map::in_bounds(int x, int y) const {
    return x >= 0 && x < MAP_W && y >= 0 && y < MAP_H;
}

bool Map::walkable(int x, int y) const {
    if (!in_bounds(x, y)) return false;
    auto t = at(x, y).type;
    return t == TileType::Floor || t == TileType::Door || t == TileType::Stairs;
}
