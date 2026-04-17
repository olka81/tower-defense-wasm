#include "game.h"
#include <cmath>
#include <algorithm>
#include <sstream>

Game::Game(int screenW, int screenH)
    : m_map(16, 12, 48),
      m_state{3, 150, 0, true, false},
      m_spawnTimer(0.0f),
      m_waveTimer(5.0f),
      m_enemiesToSpawn(0),
      m_screenW(screenW),
      m_screenH(screenH),
      m_selectedTower(TowerType::Basic) {
}

void Game::update(float dt) {
    if (!m_state.running || m_state.gameOver) return;

    if (m_enemiesToSpawn > 0) {
        m_spawnTimer -= dt;
        if (m_spawnTimer <= 0.0f) {
            Vec2 start = m_map.path.empty() ? Vec2{0, 0} : m_map.path[0];
            EnemyType t = (m_enemiesToSpawn % 3 == 0) ? EnemyType::Fast : EnemyType::Basic;
            m_enemies.push_back(Enemy(start, t));
            m_enemiesToSpawn--;
            m_spawnTimer = 1.2f;
        }
    } else {
        m_waveTimer -= dt;
        if (m_waveTimer <= 0.0f) spawnWave();
    }

    for (auto& e : m_enemies)
        e.update(dt, m_map.path);

    for (auto& t : m_towers)
        t.update(dt, m_enemies, m_projectiles);

    updateProjectiles(dt);
    checkCollisions();
    removeDeadEntities();

    for (auto& e : m_enemies) {
        if (e.reachedEnd(m_map.path)) {
            e.alive = false;
            m_state.lives--;
            if (m_state.lives <= 0) m_state.gameOver = true;
        }
    }
}

void Game::spawnWave() {
    m_state.wave++;
    m_enemiesToSpawn = 5 + m_state.wave * 3;
    m_spawnTimer = 0.0f;
    m_waveTimer = 20.0f + m_state.wave * 2.0f;
}

void Game::updateProjectiles(float dt) {
    for (auto& p : m_projectiles) {
        if (!p.active) continue;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        if (p.pos.x < 0 || p.pos.x > m_screenW ||
            p.pos.y < 0 || p.pos.y > m_screenH)
            p.active = false;
    }
}

void Game::checkCollisions() {
    for (auto& p : m_projectiles) {
        if (!p.active) continue;
        for (auto& e : m_enemies) {
            if (!e.alive) continue;
            float dx = e.pos.x - p.pos.x;
            float dy = e.pos.y - p.pos.y;
            if (std::sqrt(dx * dx + dy * dy) < 12.0f) {
                e.hp -= p.damage;
                p.active = false;
                if (e.hp <= 0.0f) {
                    e.alive = false;
                    m_state.gold += 10;
                }
                break;
            }
        }
    }
}

void Game::removeDeadEntities() {
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
                       [](const Enemy& e) { return !e.alive; }),
        m_enemies.end());
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                       [](const Projectile& p) { return !p.active; }),
        m_projectiles.end());
}

void Game::onLeftClick(int px, int py) {
    int col = px / m_map.tileSize;
    int row = py / m_map.tileSize;
    if (col < 0 || col >= m_map.cols || row < 0 || row >= m_map.rows) return;
    if (!m_map.canPlaceTower(col, row)) return;

    int cost = (m_selectedTower == TowerType::Sniper) ? 100 : 50;
    if (m_state.gold < cost) return;

    m_towers.push_back(Tower(m_map.tileCenter(col, row), m_selectedTower));
    m_map.set(col, row, Tile::Tower);
    m_state.gold -= cost;
}

void Game::onRightClick(int /*px*/, int /*py*/) {
    m_selectedTower = (m_selectedTower == TowerType::Basic)
                    ? TowerType::Sniper
                    : TowerType::Basic;
}

// ---------- Save / load -----------------------------------------------------

std::string Game::saveState() const {
    std::ostringstream o;
    o << "{\"lives\":" << m_state.lives
      << ",\"gold\":"  << m_state.gold
      << ",\"wave\":"  << m_state.wave
      << ",\"towers\":[";
    for (size_t i = 0; i < m_towers.size(); ++i) {
        if (i) o << ',';
        const auto& t = m_towers[i];
        int col = static_cast<int>(t.pos.x / m_map.tileSize);
        int row = static_cast<int>(t.pos.y / m_map.tileSize);
        o << "{\"col\":" << col
          << ",\"row\":" << row
          << ",\"type\":" << static_cast<int>(t.type) << '}';
    }
    o << "]}";
    return o.str();
}

bool Game::loadState(const std::string& json) {
    // Helper: read first integer after "key": in a string
    auto readInt = [](const std::string& src, const std::string& key, int fallback) -> int {
        const std::string needle = "\"" + key + "\":";
        auto pos = src.find(needle);
        if (pos == std::string::npos) return fallback;
        try { return std::stoi(src.substr(pos + needle.size())); }
        catch (...) { return fallback; }
    };

    const int lives = readInt(json, "lives", -1);
    const int gold  = readInt(json, "gold",  -1);
    const int wave  = readInt(json, "wave",  -1);
    if (lives < 0 || gold < 0 || wave < 0) return false;

    m_towers.clear();
    m_enemies.clear();
    m_projectiles.clear();
    m_map = Map(m_map.cols, m_map.rows, m_map.tileSize);

    m_state          = {lives, gold, wave, true, false};
    m_spawnTimer     = 0.0f;
    m_waveTimer      = 5.0f;
    m_enemiesToSpawn = 0;

    // Parse the "towers":[{...},...] array
    const std::string arrKey = "\"towers\":[";
    auto arrPos = json.find(arrKey);
    if (arrPos != std::string::npos) {
        arrPos += arrKey.size();
        const auto arrEnd = json.find(']', arrPos);
        const std::string arr = json.substr(arrPos, arrEnd - arrPos);
        std::string::size_type pos = 0;
        while ((pos = arr.find('{', pos)) != std::string::npos) {
            auto end = arr.find('}', pos);
            if (end == std::string::npos) break;
            const std::string obj = arr.substr(pos, end - pos + 1);
            pos = end + 1;

            const int col  = readInt(obj, "col",  -1);
            const int row  = readInt(obj, "row",  -1);
            const int type = readInt(obj, "type", -1);
            if (col < 0 || row < 0 || type < 0) continue;
            if (col >= m_map.cols || row >= m_map.rows) continue;
            if (!m_map.canPlaceTower(col, row)) continue;

            const TowerType tt = (type == 1) ? TowerType::Sniper : TowerType::Basic;
            m_towers.push_back(Tower(m_map.tileCenter(col, row), tt));
            m_map.set(col, row, Tile::Tower);
        }
    }
    return true;
}

// ---------- JSON render snapshot --------------------------------------------

std::string Game::getRenderData() const {
    std::ostringstream o;
    o << "{";

    // Tile grid — flat array of ints (0=Empty, 1=Path, 2=Tower)
    o << "\"cols\":" << m_map.cols << ","
      << "\"rows\":" << m_map.rows << ","
      << "\"tileSize\":" << m_map.tileSize << ","
      << "\"tiles\":[";
    for (int i = 0, n = m_map.cols * m_map.rows; i < n; ++i) {
        if (i) o << ',';
        o << static_cast<int>(m_map.grid[i]);
    }
    o << "],";

    // Towers
    o << "\"towers\":[";
    for (size_t i = 0; i < m_towers.size(); ++i) {
        if (i) o << ',';
        const auto& t = m_towers[i];
        o << "{\"x\":" << t.pos.x << ",\"y\":" << t.pos.y
          << ",\"type\":" << static_cast<int>(t.type)
          << ",\"range\":" << t.range << "}";
    }
    o << "],";

    // Enemies
    o << "\"enemies\":[";
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        if (i) o << ',';
        const auto& e = m_enemies[i];
        o << "{\"x\":" << e.pos.x << ",\"y\":" << e.pos.y
          << ",\"type\":" << static_cast<int>(e.type)
          << ",\"hp\":" << e.hp << ",\"maxHp\":" << e.maxHp << "}";
    }
    o << "],";

    // Projectiles
    o << "\"projectiles\":[";
    for (size_t i = 0; i < m_projectiles.size(); ++i) {
        if (i) o << ',';
        const auto& p = m_projectiles[i];
        o << "{\"x\":" << p.pos.x << ",\"y\":" << p.pos.y << "}";
    }
    o << "]}";

    return o.str();
}
