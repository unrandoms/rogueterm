#pragma once
#include <vector>
#include <array>
#include <cstdint>

constexpr int MAP_W = 80;
constexpr int MAP_H = 40;

enum class TileType : uint8_t {
    Wall,
    Floor,
    Door,
    Stairs
};

struct Tile {
    TileType type    = TileType::Wall;
    bool     visible = false;   // currently in FOV
    bool     seen    = false;   // has ever been seen
};

struct Rect {
    int x, y, w, h;
    int cx() const { return x + w / 2; }
    int cy() const { return y + h / 2; }
    bool intersects(const Rect& o) const {
        return x < o.x + o.w && x + w > o.x &&
               y < o.y + o.h && y + h > o.y;
    }
};

class Map {
public:
    Map();
    void generate(int floor_num);

    Tile&       at(int x, int y)       { return tiles_[y * MAP_W + x]; }
    const Tile& at(int x, int y) const { return tiles_[y * MAP_W + x]; }

    bool walkable(int x, int y) const;
    bool in_bounds(int x, int y) const;

    void reset_visibility();

    const std::vector<Rect>& rooms() const { return rooms_; }

    int player_start_x() const { return player_start_x_; }
    int player_start_y() const { return player_start_y_; }
    int stairs_x()       const { return stairs_x_; }
    int stairs_y()       const { return stairs_y_; }

private:
    std::vector<Tile> tiles_;
    std::vector<Rect> rooms_;
    int player_start_x_ = 1, player_start_y_ = 1;
    int stairs_x_ = 1, stairs_y_ = 1;

    // BSP helpers
    void bsp_split(int x, int y, int w, int h, int depth,
                   std::vector<Rect>& leaves);
    void carve_room(const Rect& r);
    void carve_corridor(int x1, int y1, int x2, int y2);
    void place_stairs();
};
