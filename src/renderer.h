#pragma once
#include <string>
#include <vector>
#include "map.h"
#include "entity.h"
#include "items.h"

constexpr int HUD_ROWS   = 6;    // message log + status bar
constexpr int TERM_W     = MAP_W;
constexpr int TERM_H     = MAP_H + HUD_ROWS;

struct Cell {
    char        ch    = ' ';
    int         color = 0;   // encoded fg color (ANSI index or 0 = default)
    bool        dim   = false;
    bool operator==(const Cell& o) const {
        return ch == o.ch && color == o.color && dim == o.dim;
    }
};

class Renderer {
public:
    Renderer();
    void init();
    void shutdown();

    // Full render: map + entities + items + HUD
    void draw(const Map& map,
              const std::vector<Entity>& entities,
              const std::vector<Item>& items,
              const std::vector<std::string>& messages,
              int floor_num,
              const std::string& equipped_item_name);

    void draw_game_over();
    void draw_victory();

private:
    std::vector<Cell> front_buf_;
    std::vector<Cell> back_buf_;

    void set_cell(int x, int y, char ch, int color, bool dim = false);
    void flush();

    static std::string ansi_color(int color, bool dim);
    void hide_cursor();
    void show_cursor();
    void move_to(int x, int y);

    // Helpers
    static Cell tile_cell(const Tile& t);
    static Cell entity_cell(const Entity& e);
    static Cell item_cell(const Item& item);
};
