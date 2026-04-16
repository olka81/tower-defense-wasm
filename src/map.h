#pragma once
#include "types.h"
#include <vector>

// Tile types
enum class Tile { Empty, Path, Tower };

struct Map {
    int cols, rows;
    int tileSize;
    std::vector<Tile> grid;
    std::vector<Vec2> path;   // world-space waypoints

    Map(int cols, int rows, int tileSize);

    Tile get(int col, int row) const;
    void set(int col, int row, Tile t);

    bool isPathTile(int col, int row) const;
    bool canPlaceTower(int col, int row) const;

    Vec2 tileCenter(int col, int row) const;

    // Builds path waypoints from the grid
    void buildPath();
};
