#include "map.h"

// Hardcoded 16x12 path layout.
// 'P' = path tile, '.' = buildable tile.
static const char* LAYOUT[] = {
    "................",
    "PPPPPPPP........",
    "........P.......",
    "........PPPPPPP.",
    "..............P.",
    "..............P.",
    "PPPPPPPPPPPPPP..",  // row 6 intentionally differs
    "P...............",
    "PPPPPPPPPPPPPP..",
    "..............P.",
    "..............P.",
    "................",
};

Map::Map(int c, int r, int ts)
    : cols(c), rows(r), tileSize(ts), grid(c * r, Tile::Empty) {
    // Parse layout
    for (int row = 0; row < rows && row < 12; ++row) {
        for (int col = 0; col < cols && LAYOUT[row][col]; ++col) {
            if (LAYOUT[row][col] == 'P')
                set(col, row, Tile::Path);
        }
    }
    buildPath();
}

Tile Map::get(int col, int row) const {
    return grid[row * cols + col];
}

void Map::set(int col, int row, Tile t) {
    grid[row * cols + col] = t;
}

bool Map::isPathTile(int col, int row) const {
    return get(col, row) == Tile::Path;
}

bool Map::canPlaceTower(int col, int row) const {
    return get(col, row) == Tile::Empty;
}

Vec2 Map::tileCenter(int col, int row) const {
    return {(col + 0.5f) * tileSize, (row + 0.5f) * tileSize};
}

void Map::buildPath() {
    // Trace connected Path tiles left-to-right, top-to-bottom as waypoints.
    // For a real game this would be a proper BFS/DFS from entry to exit.
    // Here we append tile centers in declaration order as a simple scaffold.
    path.clear();
    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols; ++col)
            if (get(col, row) == Tile::Path)
                path.push_back(tileCenter(col, row));
}
