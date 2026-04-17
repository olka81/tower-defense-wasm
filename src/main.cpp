#include <emscripten/bind.h>
#include "game.h"
#include <memory>
#include <sstream>

static std::unique_ptr<Game> g_game;

// Called once from JS after the Module is ready
void game_init(int screenW, int screenH) {
    g_game = std::make_unique<Game>(screenW, screenH);
}

// Called every frame with delta-time in seconds
void game_update(float dt) {
    if (g_game) g_game->update(dt);
}

void game_left_click(int x, int y) {
    if (g_game) g_game->onLeftClick(x, y);
}

void game_right_click(int x, int y) {
    if (g_game) g_game->onRightClick(x, y);
}

// Returns a JSON string consumed by the JS renderer each frame
std::string game_render_data() {
    if (!g_game) return "{}";
    return g_game->getRenderData();
}

// Returns a JSON string with HUD values
std::string game_state() {
    if (!g_game) return "{}";
    const GameState& s = g_game->state();
    std::ostringstream o;
    o << "{\"lives\":"   << s.lives
      << ",\"gold\":"    << s.gold
      << ",\"wave\":"    << s.wave
      << ",\"gameOver\":" << (s.gameOver ? "true" : "false")
      << ",\"selected\":\"" << (g_game->selectedTower() == TowerType::Sniper ? "Sniper" : "Basic")
      << "\"}";
    return o.str();
}

// Returns a JSON snapshot of the full game state (for Electron save/load)
std::string game_save_state() {
    if (!g_game) return "{}";
    return g_game->saveState();
}

// Restores game state from a saveState() JSON string; returns true on success
bool game_load_state(const std::string& json) {
    if (!g_game) return false;
    return g_game->loadState(json);
}

EMSCRIPTEN_BINDINGS(tower_defense) {
    emscripten::function("game_init",        &game_init);
    emscripten::function("game_update",      &game_update);
    emscripten::function("game_left_click",  &game_left_click);
    emscripten::function("game_right_click", &game_right_click);
    emscripten::function("game_render_data", &game_render_data);
    emscripten::function("game_state",       &game_state);
    emscripten::function("game_save_state",  &game_save_state);
    emscripten::function("game_load_state",  &game_load_state);
}
