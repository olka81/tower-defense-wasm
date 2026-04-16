#include "tower.h"
#include <cmath>
#include <limits>

Tower::Tower(Vec2 p, TowerType t)
    : pos(p), type(t), cooldown(0.0f) {
    switch (t) {
        case TowerType::Sniper:
            range    = 180.0f;
            damage   = 60.0f;
            fireRate = 0.5f;
            break;
        default:
            range    = 100.0f;
            damage   = 20.0f;
            fireRate = 1.5f;
    }
}

Enemy* Tower::findTarget(std::vector<Enemy>& enemies) {
    Enemy* best = nullptr;
    int bestPath = -1;

    for (auto& e : enemies) {
        if (!e.alive) continue;
        float dx = e.pos.x - pos.x;
        float dy = e.pos.y - pos.y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d <= range && e.pathIndex > bestPath) {
            bestPath = e.pathIndex;
            best = &e;
        }
    }
    return best;
}

void Tower::update(float dt, std::vector<Enemy>& enemies,
                   std::vector<Projectile>& projectiles) {
    if (cooldown > 0.0f) {
        cooldown -= dt;
        return;
    }

    Enemy* target = findTarget(enemies);
    if (!target) return;

    float dx = target->pos.x - pos.x;
    float dy = target->pos.y - pos.y;
    float d = std::sqrt(dx * dx + dy * dy);
    float speed = 200.0f;

    projectiles.push_back({pos, {(dx / d) * speed, (dy / d) * speed}, damage, true});
    cooldown = 1.0f / fireRate;
}
