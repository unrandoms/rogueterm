#pragma once
#include <string>
#include <vector>
#include "map.h"
#include "entity.h"
#include "items.h"
#include "renderer.h"
#include "input.h"

constexpr int WIN_FLOOR    = 5;
constexpr int FOV_RADIUS   = 8;
constexpr int ENEMIES_PER_FLOOR = 8;
constexpr int ITEMS_PER_FLOOR   = 5;

class Game {
public:
    explicit Game(unsigned int seed = 0);
    ~Game();
    void run();

private:
    unsigned int seed_;
    int          floor_num_     = 1;
    bool         running_       = true;
    bool         player_dead_   = false;
    bool         player_won_    = false;

    Map                   map_;
    std::vector<Entity>   entities_;   // entities_[0] is always the player
    std::vector<Item>     items_;
    std::vector<std::string> messages_;

    Renderer renderer_;

    // Equipped item name (empty = none)
    std::string equipped_name_;
    int         extra_attack_  = 0;
    int         extra_defense_ = 0;

    void load_floor();
    void do_player_move(int dx, int dy);
    void do_player_pickup();
    void do_enemy_turns();
    void recompute_fov();
    void push_message(const std::string& msg);
    void apply_item(const Item& item);
    Entity& player() { return entities_[0]; }
};
