#include "game.h"
#include "fov.h"
#include "combat.h"
#include "rng.h"
#include <algorithm>
#include <cmath>
#include <chrono>

Game::Game(unsigned int seed) : seed_(seed) {
    g_rng.seed(seed_ ? seed_ : static_cast<unsigned int>(
        std::chrono::steady_clock::now().time_since_epoch().count()));
}

Game::~Game() {}

void Game::push_message(const std::string& msg) {
    messages_.push_back(msg);
    if (messages_.size() > 64) messages_.erase(messages_.begin());
}

void Game::load_floor() {
    map_.generate(floor_num_);
    items_.clear();

    // Keep player alive, reset position
    int px = map_.player_start_x();
    int py = map_.player_start_y();

    if (entities_.empty()) {
        entities_.push_back(Entity::make_player(px, py));
    } else {
        entities_[0].x = px;
        entities_[0].y = py;
    }

    // Remove old enemies
    entities_.erase(entities_.begin() + 1, entities_.end());

    // Scatter enemies (more per floor)
    int enemy_count = ENEMIES_PER_FLOOR + (floor_num_ - 1) * 2;
    scatter_enemies(entities_, map_, enemy_count, floor_num_);

    // Scatter items
    int item_count = ITEMS_PER_FLOOR + floor_num_ / 2;
    scatter_items(items_, map_, item_count);

    recompute_fov();
    push_message("You descend to floor " + std::to_string(floor_num_) + ".");
}

void Game::recompute_fov() {
    compute_fov(map_, player().x, player().y, FOV_RADIUS);
}

void Game::do_player_move(int dx, int dy) {
    int nx = player().x + dx;
    int ny = player().y + dy;

    if (!map_.in_bounds(nx, ny)) return;

    // Check for enemy at target
    for (auto& e : entities_) {
        if (!e.alive || e.is_player()) continue;
        if (e.x == nx && e.y == ny) {
            // Apply equipment bonuses temporarily
            int orig_attack  = player().attack;
            int orig_defense = player().defense;
            player().attack  += extra_attack_;
            player().defense += extra_defense_;

            auto result = resolve_attack(player(), e);
            player().attack  = orig_attack;
            player().defense = orig_defense;

            push_message(result.message);
            do_enemy_turns();
            recompute_fov();
            return;
        }
    }

    // Check walkable
    if (!map_.walkable(nx, ny)) return;

    // Move
    player().x = nx;
    player().y = ny;

    // Check stairs
    if (map_.at(nx, ny).type == TileType::Stairs) {
        if (floor_num_ >= WIN_FLOOR) {
            player_won_ = true;
            running_    = false;
            return;
        }
        floor_num_++;
        load_floor();
        return;
    }

    do_enemy_turns();
    recompute_fov();
}

void Game::do_player_pickup() {
    int px = player().x;
    int py = player().y;

    for (auto& item : items_) {
        if (item.x == px && item.y == py) {
            push_message("You pick up the " + item.name + ".");
            apply_item(item);
            item.x = -1; item.y = -1;  // remove from map
            do_enemy_turns();
            recompute_fov();
            return;
        }
    }
    push_message("Nothing to pick up here.");
}

void Game::apply_item(const Item& item) {
    switch (item.type) {
        case ItemType::HealthPotion: {
            int healed = std::min(item.value, player().max_hp - player().hp);
            player().hp += healed;
            push_message("You drink the potion and recover " +
                         std::to_string(healed) + " HP.");
            break;
        }
        case ItemType::Sword:
            // Replace previous weapon bonus
            extra_attack_  = item.value;
            equipped_name_ = item.name;
            push_message("You wield the " + item.name +
                         " (+"+std::to_string(item.value)+" ATK).");
            break;
        case ItemType::Shield:
            extra_defense_ = item.value;
            equipped_name_ = item.name;
            push_message("You equip the " + item.name +
                         " (+"+std::to_string(item.value)+" DEF).");
            break;
    }
}

void Game::do_enemy_turns() {
    int px = player().x;
    int py = player().y;

    for (auto& e : entities_) {
        if (!e.alive || e.is_player()) continue;

        e.tick_speed();
        if (!e.can_move()) continue;

        // Determine behavior per type
        bool can_see_player = map_.at(px, py).visible;  // proxy: player in FOV
        int dist = std::abs(e.x - px) + std::abs(e.y - py);

        int nx = e.x, ny = e.y;

        switch (e.type) {
            case EntityType::Rat:
                if (can_see_player) {
                    // 70% chase, 30% random
                    if (g_rng.chance(70)) {
                        auto [bx, by] = bfs_step(map_, entities_, e.x, e.y, px, py);
                        nx = bx; ny = by;
                    } else {
                        static const int dx4[] = {0,0,-1,1};
                        static const int dy4[] = {-1,1,0,0};
                        int d = g_rng.range(0, 3);
                        int rx = e.x + dx4[d], ry = e.y + dy4[d];
                        if (map_.walkable(rx, ry)) { nx = rx; ny = ry; }
                    }
                } else {
                    // Random wander
                    static const int dx4[] = {0,0,-1,1};
                    static const int dy4[] = {-1,1,0,0};
                    int d = g_rng.range(0, 3);
                    int rx = e.x + dx4[d], ry = e.y + dy4[d];
                    if (map_.walkable(rx, ry)) { nx = rx; ny = ry; }
                }
                break;

            case EntityType::Goblin:
                if (can_see_player) {
                    auto [bx, by] = bfs_step(map_, entities_, e.x, e.y, px, py);
                    nx = bx; ny = by;
                } else {
                    // Wander
                    static const int dx4[] = {0,0,-1,1};
                    static const int dy4[] = {-1,1,0,0};
                    int d = g_rng.range(0, 3);
                    int rx = e.x + dx4[d], ry = e.y + dy4[d];
                    if (map_.walkable(rx, ry)) { nx = rx; ny = ry; }
                }
                break;

            case EntityType::Orc:
                if (dist <= 5) {
                    auto [bx, by] = bfs_step(map_, entities_, e.x, e.y, px, py);
                    nx = bx; ny = by;
                }
                // else rest
                break;

            default: break;
        }

        // Attack if adjacent to player
        if (nx == px && ny == py) {
            int orig_attack  = e.attack;
            int orig_defense = e.defense;
            // enemies don't have equipment bonuses
            (void)orig_attack; (void)orig_defense;

            // Apply player defense bonus from equipment
            int saved_def = player().defense;
            player().defense += extra_defense_;
            auto result = resolve_attack(e, player());
            player().defense = saved_def;

            push_message(result.message);
            if (!player().alive) {
                player_dead_ = true;
                running_     = false;
                return;
            }
        } else {
            // Check if destination is clear of other entities
            bool blocked = false;
            for (auto& other : entities_) {
                if (!other.alive) continue;
                if (other.x == nx && other.y == ny) { blocked = true; break; }
            }
            if (!blocked && map_.walkable(nx, ny)) {
                e.x = nx; e.y = ny;
            }
        }
    }
}

void Game::run() {
    renderer_.init();
    input_init();

    load_floor();
    push_message("Welcome to Rogueterm! Navigate with WASD/hjkl. Pick up items with g.");

    while (running_) {
        renderer_.draw(map_, entities_, items_, messages_, floor_num_, equipped_name_);

        Key k = read_key();
        switch (k) {
            case Key::Up:    do_player_move( 0, -1); break;
            case Key::Down:  do_player_move( 0,  1); break;
            case Key::Left:  do_player_move(-1,  0); break;
            case Key::Right: do_player_move( 1,  0); break;
            case Key::PickUp: do_player_pickup(); break;
            case Key::Wait:
                do_enemy_turns();
                recompute_fov();
                break;
            case Key::Quit:
                running_ = false;
                break;
            default: break;
        }
    }

    // Final screen
    if (player_won_) {
        renderer_.draw_victory();
        read_key();
    } else if (player_dead_) {
        renderer_.draw_game_over();
        read_key();
    }

    input_restore();
    renderer_.shutdown();
}
