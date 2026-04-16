#pragma once
#include "types.h"
#include <vector>

struct Enemy {
    Vec2 pos;
    EnemyType type;
    float hp;
    float maxHp;
    float speed;
    int pathIndex;   // current waypoint index
    bool alive;

    Enemy(Vec2 startPos, EnemyType t);
    void update(float dt, const std::vector<Vec2>& path);
    bool reachedEnd(const std::vector<Vec2>& path) const;
};
