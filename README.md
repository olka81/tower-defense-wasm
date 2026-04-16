# Tower Defense (C++ → WebAssembly)

A minimalist tower defense game written in C++17, compiled to WebAssembly via
Emscripten. **No third-party libraries.** The C++ core handles all game logic
and exposes a thin API via Emscripten bindings; JavaScript owns the render loop
using the HTML5 Canvas API.

```
C++ (game logic)  ──emscripten/bind.h──►  WASM module
                                               │
                                          game.js (RAF loop)
                                               │
                                          Canvas 2D API
```

## Prerequisites

| Tool | Version |
|------|---------|
| [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) | 3.1+ |
| CMake | 3.20+ |
| A modern browser | Chrome / Firefox / Safari |

## Build

```bash
# Activate the Emscripten toolchain (adjust path as needed)
source ~/emsdk/emsdk_env.sh

# Configure
emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Compile — outputs web/tower_defense.js + web/tower_defense.wasm
cmake --build build --parallel
```

## Run

Browsers block WASM loaded from `file://`, so serve over HTTP:

```bash
python3 -m http.server 8080 --directory web
# Open http://localhost:8080
```

## Controls

| Input | Action |
|-------|--------|
| Left-click on grass tile | Place selected tower |
| Right-click anywhere | Toggle tower type (Basic ↔ Sniper) |

## Project layout

```
tower-defense-wasm/
├── src/
│   ├── types.h          # Shared POD types (Vec2, TowerType, EnemyType)
│   ├── enemy.h/.cpp     # Enemy: waypoint following, HP, speed
│   ├── tower.h/.cpp     # Tower: targeting, fire-rate, projectile spawning
│   ├── map.h/.cpp       # Tile grid (16×12) + waypoint path
│   ├── game.h/.cpp      # Game loop, wave logic, getRenderData() JSON
│   └── main.cpp         # Emscripten bindings (no SDL, no window)
├── web/
│   ├── index.html       # Page shell — loads WASM module + game.js
│   ├── game.js          # RAF loop, Canvas 2D renderer, HUD updates
│   └── style.css        # Dark-theme page styles
├── build/               # CMake output (git-ignored)
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## WASM API (emscripten/bind.h)

| JS call | Description |
|---------|-------------|
| `Module.game_init(w, h)` | Create world (call once after WASM loads) |
| `Module.game_update(dt)` | Advance simulation by `dt` seconds |
| `Module.game_left_click(x, y)` | Place tower at canvas pixel |
| `Module.game_right_click(x, y)` | Cycle selected tower type |
| `Module.game_render_data()` | JSON snapshot of tiles / towers / enemies / projectiles |
| `Module.game_state()` | JSON with `lives`, `gold`, `wave`, `gameOver`, `selected` |

## Tower types

| Tower | Cost | Range | Damage | Fire rate |
|-------|------|-------|--------|-----------|
| Basic | 50g | 100 px | 20 | 1.5 /s |
| Sniper | 100g | 180 px | 60 | 0.5 /s |

## Enemy types

| Enemy | HP | Speed |
|-------|----|-------|
| Basic | 100 | 50 px/s |
| Fast | 60 | 90 px/s |
