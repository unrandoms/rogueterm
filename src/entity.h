#pragma once
#include <string>
#include <vector>

enum class EntityType {
    Player,
    Rat,
    Goblin,
    Orc
};

struct Entity {
    EntityType  type;
    std::string name;
    int         x, y;
    int         hp, max_hp;
    int         attack;
    int         defense;
    char        glyph;
    bool        alive = true;

    // Only valid for enemies: cooldown ticks between moves (1 = every turn)
    int         speed_ticks = 1;
    int         speed_counter = 0;

    bool is_player() const { return type == EntityType::Player; }
    bool can_move()  const {
        if (speed_ticks <= 1) return true;
        return speed_counter <= 0;
    }
    void tick_speed() {
        if (speed_ticks > 1) {
            if (speed_counter > 0) --speed_counter;
            else speed_counter = speed_ticks - 1;
        }
    }

    static Entity make_player(int x, int y);
    static Entity make_rat(int x, int y);
    static Entity make_goblin(int x, int y);
    static Entity make_orc(int x, int y);
};

class Map;
struct Entity;

// BFS pathfinding: returns next step (nx, ny) toward (tx, ty) or stays put.
std::pair<int,int> bfs_step(const Map& map,
                             const std::vector<Entity>& entities,
                             int sx, int sy, int tx, int ty);

// Scatter enemies into the map
void scatter_enemies(std::vector<Entity>& entities, const Map& map,
                     int count, int floor_num);
