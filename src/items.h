#pragma once
#include <string>
#include <vector>

enum class ItemType {
    HealthPotion,
    Sword,
    Shield
};

struct Item {
    ItemType    type;
    std::string name;
    int         value;   // HP restored / attack bonus / defense bonus
    int         x, y;   // world position (-1,-1 if in inventory)
    char        glyph;

    static Item make_health_potion(int x, int y);
    static Item make_sword(int x, int y);
    static Item make_shield(int x, int y);
};

// Randomly scatter items in the given rooms (floor coords)
struct Rect;
class Map;
void scatter_items(std::vector<Item>& items, const Map& map, int count);
