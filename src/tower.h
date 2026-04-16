#pragma once
#include "types.h"
#include "enemy.h"
#include <vector>

struct Projectile {
    Vec2 pos;
    Vec2 vel;
    float damage;
    bool active;
};

struct Tower {
    Vec2 pos;
    TowerType type;
    float range;
    float damage;
    float fireRate;    // shots per second
    float cooldown;    // time until next shot

    Tower(Vec2 p, TowerType t);
    void update(float dt, std::vector<Enemy>& enemies,
                std::vector<Projectile>& projectiles);

private:
    Enemy* findTarget(std::vector<Enemy>& enemies);
};
