#pragma once
#include "map.h"
#include "tower.h"
#include "enemy.h"
#include <string>
#include <vector>

struct GameState {
    int lives;
    int gold;
    int wave;
    bool running;
    bool gameOver;
};

class Game {
public:
    Game(int screenW, int screenH);

    void update(float dt);

    void onLeftClick(int pixelX, int pixelY);
    void onRightClick(int pixelX, int pixelY);

    // Returns JSON snapshot of all renderable entities for the JS layer
    std::string getRenderData() const;

    const GameState& state() const { return m_state; }
    TowerType selectedTower() const { return m_selectedTower; }

private:
    void spawnWave();
    void updateProjectiles(float dt);
    void checkCollisions();
    void removeDeadEntities();

    Map m_map;
    std::vector<Tower> m_towers;
    std::vector<Enemy> m_enemies;
    std::vector<Projectile> m_projectiles;

    GameState m_state;
    float m_spawnTimer;
    float m_waveTimer;
    int m_enemiesToSpawn;

    int m_screenW, m_screenH;
    TowerType m_selectedTower;
};
