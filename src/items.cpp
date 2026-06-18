#include "items.h"
#include "map.h"
#include "rng.h"

Item Item::make_health_potion(int x, int y) {
    return { ItemType::HealthPotion, "Health Potion", 20, x, y, '!' };
}

Item Item::make_sword(int x, int y) {
    return { ItemType::Sword, "Iron Sword", 5, x, y, ')' };
}

Item Item::make_shield(int x, int y) {
    return { ItemType::Shield, "Wooden Shield", 3, x, y, '[' };
}

void scatter_items(std::vector<Item>& items, const Map& map, int count) {
    // Collect all walkable, non-stairs floor tiles
    std::vector<std::pair<int,int>> floors;
    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x) {
            auto t = map.at(x, y).type;
            if (t == TileType::Floor)
                floors.emplace_back(x, y);
        }
    if (floors.empty()) return;

    for (int i = 0; i < count; ++i) {
        auto [fx, fy] = floors[g_rng.range(0, static_cast<int>(floors.size()) - 1)];
        int roll = g_rng.range(1, 3);
        if (roll == 1)      items.push_back(Item::make_health_potion(fx, fy));
        else if (roll == 2) items.push_back(Item::make_sword(fx, fy));
        else                items.push_back(Item::make_shield(fx, fy));
    }
}
