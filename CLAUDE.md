# CLAUDE.md — Tower Defense (C++ → WebAssembly)

## Project overview

Minimalistic Tower Defense game written in C++17 and compiled to WebAssembly
via Emscripten. The C++ core is deliberately kept free of all commercial and
third-party libraries — no SDL2, no OpenGL, no external game frameworks. All
rendering is done in vanilla JavaScript using the HTML5 Canvas API. The game
runs entirely in the browser with no server-side logic.

## Tech stack

| Layer | Technology |
|-------|-----------|
| Game logic | C++17 |
| Compilation to WASM | Emscripten 5.0.6 |
| C++ → JS bridge | `emscripten/bind.h` (`--bind` linker flag) |
| Rendering | HTML5 Canvas 2D API (vanilla JS) |
| Build system | CMake 3.20+ (`emcmake`) |
| CI/CD | GitHub Actions *(planned)* |
| Hosting | GitHub Pages *(planned)* |

## Project structure

```
tower-defense-wasm/
├── src/
│   ├── types.h          # Shared POD types: Vec2, TowerType, EnemyType
│   ├── enemy.h/.cpp     # Enemy: waypoint following, HP, speed variants
│   ├── tower.h/.cpp     # Tower: target acquisition, fire rate, projectile firing
│   ├── map.h/.cpp       # 16×12 tile grid + hardcoded waypoint path
│   ├── game.h/.cpp      # Top-level game loop, wave spawning, getRenderData()
│   └── main.cpp         # Emscripten bindings only — no window, no SDL
├── web/
│   ├── index.html       # Page shell; loads tower_defense.js + game.js
│   ├── game.js          # requestAnimationFrame loop + Canvas 2D renderer
│   └── style.css        # Minimal dark-theme styles
├── .vscode/
│   └── tasks.json       # Build (Release/Debug), Serve, Build & Serve
├── build/               # CMake output — git-ignored
├── build.sh             # One-shot: source emsdk → cmake configure → build
├── serve.sh             # python3 http.server on port 8080 from web/
├── CMakeLists.txt       # Emscripten-only build (fails fast if not emcmake)
├── .gitignore
└── README.md
```

## Build instructions

```bash
# First time only — install Emscripten
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
echo 'source "$HOME/emsdk/emsdk_env.sh"' >> ~/.bashrc
source ~/emsdk/emsdk_env.sh

# Build (outputs web/tower_defense.js + web/tower_defense.wasm)
cd ~/projects/tower-defense-wasm
./build.sh               # Release (default)
./build.sh Debug         # Debug build

# Serve locally (browsers block WASM on file://)
./serve.sh               # http://localhost:8080
./serve.sh 9000          # custom port
```

From VS Code: **Ctrl+Shift+B** runs the Release build. Use the task picker
(`Ctrl+Shift+P` → "Run Task") for **Build & Serve** or **Serve (port 8080)**.

## Architecture

```
C++ (pure game logic — no I/O, no rendering)
  ├── Map       — tile grid, waypoint list
  ├── Enemy     — position, HP, pathfinding along waypoints
  ├── Tower     — targeting, cooldown, projectile spawning
  └── Game      — update loop, wave timer, collision, getRenderData()
         │
         │  emscripten/bind.h  (--bind, MODULARIZE=1)
         ▼
  tower_defense.wasm  +  tower_defense.js  (generated)
         │
         │  Module.game_render_data() → JSON
         │  Module.game_state()       → JSON
         ▼
  game.js  (requestAnimationFrame loop)
    ├── calls Module.game_update(dt) each frame
    ├── parses JSON snapshot from Module.game_render_data()
    ├── draws tiles / towers / enemies / projectiles via Canvas 2D
    └── updates HUD DOM elements (lives, gold, wave, selected tower)
```

### WASM API surface

| JS call | Description |
|---------|-------------|
| `Module.game_init(w, h)` | Create world — call once after WASM is ready |
| `Module.game_update(dt)` | Advance simulation by `dt` seconds |
| `Module.game_left_click(x, y)` | Place selected tower at canvas pixel |
| `Module.game_right_click(x, y)` | Cycle selected tower type |
| `Module.game_render_data()` | JSON: tiles, towers, enemies, projectiles |
| `Module.game_state()` | JSON: lives, gold, wave, gameOver, selected |

## Current status (MVP)

Working:
- 16×12 tile map with a hardcoded path and buildable grass tiles
- Two tower types: **Basic** (50g, range 100, 1.5/s) and **Sniper** (100g, range 180, 0.5/s)
- Two enemy types: **Basic** (100 HP, 50 px/s) and **Fast** (60 HP, 90 px/s)
- Wave system: waves grow in size each round; enemies spawn at fixed interval
- Tower targeting: nearest enemy furthest along the path
- Projectile collision with hit radius
- Gold reward on kill; life deducted when enemy reaches end
- HUD: lives, gold, wave counter, selected tower indicator
- Game Over screen on canvas

## Next steps

- **CI/CD** — GitHub Actions workflow that runs `./build.sh` on every push to `main`
- **GitHub Pages** — deploy `web/` automatically after a successful CI build
- Proper BFS path-finding instead of the hardcoded row-order waypoints in `map.cpp`
- Tower sell / upgrade mechanics
- Sound effects via Web Audio API (no libraries)
- Mobile touch input support

## Code conventions

- **C++17** — use structured bindings, `if constexpr`, `std::optional` where
  they improve clarity; avoid C++20 features until Emscripten support is verified
- **No external dependencies** — zero third-party or commercial libraries;
  standard library and `emscripten/bind.h` only
- **No SDL, no OpenGL** — all rendering belongs in `web/game.js`
- **No raw `new`/`delete`** — use `std::unique_ptr` / `std::make_unique`
- **Game logic is renderer-agnostic** — C++ classes must not know about the
  canvas or DOM; the only output path is `getRenderData()` / `game_state()`
- JSON serialised in `getRenderData()` by hand (`<sstream>`) — no JSON library
- JS is plain ES2017 (`'use strict'`, no bundler, no framework, no TypeScript)
