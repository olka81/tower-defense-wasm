#include "enemy.h"
#include <cmath>

namespace {
float dist(Vec2 a, Vec2 b) {
    float dx = b.x - a.x, dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}
}

Enemy::Enemy(Vec2 startPos, EnemyType t)
    : pos(startPos), type(t), pathIndex(0), alive(true) {
    switch (t) {
        case EnemyType::Fast:
            hp = maxHp = 60.0f;
            speed = 90.0f;
            break;
        default:
            hp = maxHp = 100.0f;
            speed = 50.0f;
    }
}

void Enemy::update(float dt, const std::vector<Vec2>& path) {
    if (!alive || pathIndex >= static_cast<int>(path.size())) return;

    Vec2 target = path[pathIndex];
    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    float d = std::sqrt(dx * dx + dy * dy);

    if (d < 2.0f) {
        pathIndex++;
    } else {
        float step = speed * dt;
        pos.x += (dx / d) * step;
        pos.y += (dy / d) * step;
    }

    if (hp <= 0.0f) alive = false;
}

bool Enemy::reachedEnd(const std::vector<Vec2>& path) const {
    return pathIndex >= static_cast<int>(path.size());
}
