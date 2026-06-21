#include "renderer.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cassert>

// ANSI color indices
static constexpr int COL_DEFAULT     =  0;
static constexpr int COL_WHITE       = 37;
static constexpr int COL_DARK_GREY   = 90;
static constexpr int COL_LIGHT_GREY  = 37;
static constexpr int COL_YELLOW      = 33;
static constexpr int COL_GREEN       = 32;
static constexpr int COL_RED         = 31;
static constexpr int COL_CYAN        = 36;
static constexpr int COL_BLUE        = 34;
static constexpr int COL_BROWN       = 33;  // same as yellow, bold
static constexpr int COL_BRIGHT_WHITE= 97;

Renderer::Renderer()
    : front_buf_(TERM_W * TERM_H),
      back_buf_(TERM_W * TERM_H) {}

void Renderer::init() {
    // Enter alternate screen, hide cursor
    std::fputs("\033[?1049h", stdout);
    hide_cursor();
    std::fflush(stdout);
    // Invalidate front buffer to force full redraw
    for (auto& c : front_buf_) c.ch = 0;
}

void Renderer::shutdown() {
    show_cursor();
    std::fputs("\033[?1049l", stdout);
    std::fflush(stdout);
}

void Renderer::hide_cursor() { std::fputs("\033[?25l", stdout); }
void Renderer::show_cursor() { std::fputs("\033[?25h", stdout); }

void Renderer::move_to(int x, int y) {
    // ANSI rows/cols are 1-indexed
    char buf[32];
    std::snprintf(buf, sizeof(buf), "\033[%d;%dH", y + 1, x + 1);
    std::fputs(buf, stdout);
}

std::string Renderer::ansi_color(int color, bool dim) {
    if (color == COL_DEFAULT && !dim) return "\033[0m";
    char buf[32];
    if (dim)
        std::snprintf(buf, sizeof(buf), "\033[2;%dm", color);
    else
        std::snprintf(buf, sizeof(buf), "\033[0;%dm", color);
    return buf;
}

void Renderer::set_cell(int x, int y, char ch, int color, bool dim) {
    if (x < 0 || x >= TERM_W || y < 0 || y >= TERM_H) return;
    back_buf_[y * TERM_W + x] = { ch, color, dim };
}

Cell Renderer::tile_cell(const Tile& t) {
    if (!t.visible && !t.seen) return {' ', COL_DEFAULT, false};
    bool dim = !t.visible;
    switch (t.type) {
        case TileType::Floor:  return {'.', COL_DARK_GREY, dim};
        case TileType::Wall:   return {'#', COL_LIGHT_GREY, dim};
        case TileType::Door:   return {'+', COL_BROWN, dim};
        case TileType::Stairs: return {'>', COL_BRIGHT_WHITE, dim};
    }
    return {' ', COL_DEFAULT, false};
}

Cell Renderer::entity_cell(const Entity& e) {
    switch (e.type) {
        case EntityType::Player:  return {'@', COL_WHITE, false};
        case EntityType::Rat:     return {'r', COL_YELLOW, false};
        case EntityType::Goblin:  return {'g', COL_GREEN, false};
        case EntityType::Orc:     return {'O', COL_RED, false};
    }
    return {'?', COL_WHITE, false};
}

Cell Renderer::item_cell(const Item& item) {
    switch (item.type) {
        case ItemType::HealthPotion: return {'!', COL_RED, false};
        case ItemType::Sword:        return {')', COL_CYAN, false};
        case ItemType::Shield:       return {'[', COL_BLUE, false};
    }
    return {'?', COL_WHITE, false};
}

void Renderer::draw(const Map& map,
                    const std::vector<Entity>& entities,
                    const std::vector<Item>& items,
                    const std::vector<std::string>& messages,
                    int floor_num,
                    const std::string& equipped_item_name) {
    // Clear back buffer
    for (auto& c : back_buf_) c = {' ', COL_DEFAULT, false};

    // 1. Map tiles
    for (int y = 0; y < MAP_H; ++y)
        for (int x = 0; x < MAP_W; ++x)
            set_cell(x, y, tile_cell(map.at(x, y)).ch,
                           tile_cell(map.at(x, y)).color,
                           tile_cell(map.at(x, y)).dim);

    // 2. Items (only if visible)
    for (auto& item : items) {
        if (item.x < 0) continue;  // picked up
        if (!map.at(item.x, item.y).visible) continue;
        auto c = item_cell(item);
        set_cell(item.x, item.y, c.ch, c.color, false);
    }

    // 3. Entities (only if visible)
    for (auto& e : entities) {
        if (!e.alive) continue;
        if (!map.at(e.x, e.y).visible) continue;
        auto c = entity_cell(e);
        set_cell(e.x, e.y, c.ch, c.color, false);
    }

    // 4. HUD: separator line
    for (int x = 0; x < TERM_W; ++x)
        set_cell(x, MAP_H, '-', COL_DARK_GREY, false);

    // 5. Stats line
    const Entity& player = entities[0];
    int hp_bar_max = 20;
    int hp_filled  = (player.max_hp > 0)
                   ? hp_bar_max * player.hp / player.max_hp
                   : 0;
    std::string hp_bar = "[";
    for (int i = 0; i < hp_bar_max; ++i)
        hp_bar += (i < hp_filled) ? '#' : ' ';
    hp_bar += "]";

    std::string stats = "HP: " + hp_bar +
                        " " + std::to_string(player.hp) + "/" +
                        std::to_string(player.max_hp) +
                        "  Floor:" + std::to_string(floor_num) +
                        "  Equipped:" + (equipped_item_name.empty() ? "None" : equipped_item_name);
    for (int x = 0; x < std::min((int)stats.size(), TERM_W); ++x)
        set_cell(x, MAP_H + 1, stats[x], COL_WHITE, false);

    // 6. Message log (last 4)
    int log_start = std::max(0, (int)messages.size() - 4);
    for (int i = 0; i < 4; ++i) {
        int row = MAP_H + 2 + i;
        if (log_start + i < (int)messages.size()) {
            const std::string& msg = messages[log_start + i];
            for (int x = 0; x < std::min((int)msg.size(), TERM_W); ++x)
                set_cell(x, row, msg[x], COL_DEFAULT, false);
        }
    }

    flush();
}

void Renderer::flush() {
    int last_color = -1;
    bool last_dim  = false;

    for (int y = 0; y < TERM_H; ++y) {
        for (int x = 0; x < TERM_W; ++x) {
            int idx = y * TERM_W + x;
            const Cell& bc = back_buf_[idx];
            const Cell& fc = front_buf_[idx];
            if (bc == fc) continue;

            // Move cursor
            move_to(x, y);

            // Color change?
            if (bc.color != last_color || bc.dim != last_dim) {
                std::fputs(ansi_color(bc.color, bc.dim).c_str(), stdout);
                last_color = bc.color;
                last_dim   = bc.dim;
            }
            std::fputc(bc.ch, stdout);
            front_buf_[idx] = bc;
        }
    }
    // Reset color at end
    std::fputs("\033[0m", stdout);
    std::fflush(stdout);
}

void Renderer::draw_game_over() {
    // Clear back buffer
    for (auto& c : back_buf_) c = {' ', COL_DEFAULT, false};

    auto write_centered = [&](int row, const std::string& text, int color) {
        int cx = (TERM_W - static_cast<int>(text.size())) / 2;
        for (int i = 0; i < (int)text.size() && cx + i < TERM_W; ++i)
            set_cell(cx + i, row, text[i], color, false);
    };

    write_centered(TERM_H / 2 - 2, "  ***  GAME OVER  ***  ", COL_RED);
    write_centered(TERM_H / 2,     "You have perished in the dungeon.", COL_WHITE);
    write_centered(TERM_H / 2 + 2, "Press any key to exit.", COL_DARK_GREY);
    flush();
}

void Renderer::draw_victory() {
    for (auto& c : back_buf_) c = {' ', COL_DEFAULT, false};

    auto write_centered = [&](int row, const std::string& text, int color) {
        int cx = (TERM_W - static_cast<int>(text.size())) / 2;
        for (int i = 0; i < (int)text.size() && cx + i < TERM_W; ++i)
            set_cell(cx + i, row, text[i], color, false);
    };

    write_centered(TERM_H / 2 - 2, "  ***  YOU WIN!  ***  ", COL_GREEN);
    write_centered(TERM_H / 2,     "You escaped the dungeon alive!", COL_WHITE);
    write_centered(TERM_H / 2 + 2, "Press any key to exit.", COL_DARK_GREY);
    flush();
}
