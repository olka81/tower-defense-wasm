#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EMSDK="$HOME/emsdk"
BUILD_DIR="$SCRIPT_DIR/build"
BUILD_TYPE="${1:-Release}"

# ---- 1. Source emsdk -------------------------------------------------------
if [[ ! -f "$EMSDK/emsdk_env.sh" ]]; then
  echo "ERROR: emsdk not found at $EMSDK" >&2
  echo "       Run: git clone https://github.com/emscripten-core/emsdk.git ~/emsdk && cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest" >&2
  exit 1
fi
# shellcheck source=/dev/null
source "$EMSDK/emsdk_env.sh" --quiet

# ---- 2. Configure ----------------------------------------------------------
echo "==> Configuring (${BUILD_TYPE})…"
emcmake cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# ---- 3. Build --------------------------------------------------------------
echo "==> Building…"
cmake --build "$BUILD_DIR" --parallel

echo ""
echo "==> Done. Output:"
ls -lh "$SCRIPT_DIR/web/tower_defense.js" "$SCRIPT_DIR/web/tower_defense.wasm"
echo ""
echo "Serve with:  python3 -m http.server 8080 --directory web"
