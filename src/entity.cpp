#include "entity.h"
#include "map.h"
#include "rng.h"
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <cmath>

Entity Entity::make_player(int x, int y) {
    return { EntityType::Player, "Player", x, y, 30, 30, 5, 2, '@', true, 1, 0 };
}

Entity Entity::make_rat(int x, int y) {
    return { EntityType::Rat, "Rat", x, y, 6, 6, 3, 1, 'r', true, 1, 0 };
}

Entity Entity::make_goblin(int x, int y) {
    return { EntityType::Goblin, "Goblin", x, y, 12, 12, 5, 2, 'g', true, 1, 0 };
}

Entity Entity::make_orc(int x, int y) {
    // Orcs are slow — move every 2 turns
    return { EntityType::Orc, "Orc", x, y, 20, 20, 8, 4, 'O', true, 2, 0 };
}

// BFS that avoids walls and other entity positions (except target).
std::pair<int,int> bfs_step(const Map& map,
                             const std::vector<Entity>& entities,
                             int sx, int sy, int tx, int ty) {
    if (sx == tx && sy == ty) return {sx, sy};

    // Build blocked set from all alive entities except the one at start
    auto blocked = [&](int x, int y) {
        if (!map.walkable(x, y)) return true;
        for (auto& e : entities)
            if (e.alive && e.x == x && e.y == y && !(x == tx && y == ty))
                return true;
        return false;
    };

    struct Pos { int x, y; };
    auto encode = [](int x, int y) { return y * MAP_W + x; };

    std::queue<Pos> q;
    std::unordered_map<int, Pos> parent;
    int start_key = encode(sx, sy);
    int goal_key  = encode(tx, ty);
    parent[start_key] = {-1, -1};
    q.push({sx, sy});

    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};

    while (!q.empty()) {
        auto [cx, cy] = q.front(); q.pop();
        int ck = encode(cx, cy);
        if (ck == goal_key) {
            // Trace back to find first step
            Pos cur = {cx, cy};
            Pos prev = parent.at(encode(cx, cy));
            while (!(prev.x == sx && prev.y == sy)) {
                cur  = prev;
                prev = parent.at(encode(prev.x, prev.y));
            }
            return {cur.x, cur.y};
        }
        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            int nk = encode(nx, ny);
            if (!map.in_bounds(nx, ny)) continue;
            if (parent.count(nk)) continue;
            if (blocked(nx, ny) && !(nx == tx && ny == ty)) continue;
            parent[nk] = {cx, cy};
            q.push({nx, ny});
        }
    }
    // No path found — stay
    return {sx, sy};
}

void scatter_enemies(std::vector<Entity>& entities, const Map& map,
                     int count, int floor_num) {
    std::vector<std::pair<int,int>> floors;
    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x)
            if (map.at(x, y).type == TileType::Floor)
                floors.emplace_back(x, y);

    if (floors.empty()) return;

    // Player is always entities[0]
    for (int i = 0; i < count; ++i) {
        int idx;
        int tries = 0;
        do {
            idx = g_rng.range(0, static_cast<int>(floors.size()) - 1);
            ++tries;
        } while (tries < 50 && (
            (std::abs(floors[idx].first - entities[0].x) < 5 &&
             std::abs(floors[idx].second - entities[0].y) < 5)));

        auto [fx, fy] = floors[idx];

        // Weight distribution scales with floor
        int roll = g_rng.range(1, 10);
        Entity e;
        if (floor_num <= 2) {
            e = (roll <= 7) ? Entity::make_rat(fx, fy) : Entity::make_goblin(fx, fy);
        } else if (floor_num == 3) {
            if (roll <= 4)      e = Entity::make_rat(fx, fy);
            else if (roll <= 8) e = Entity::make_goblin(fx, fy);
            else                e = Entity::make_orc(fx, fy);
        } else {
            if (roll <= 2)      e = Entity::make_rat(fx, fy);
            else if (roll <= 6) e = Entity::make_goblin(fx, fy);
            else                e = Entity::make_orc(fx, fy);
        }
        entities.push_back(e);
    }
}
