#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WEB_DIR="$SCRIPT_DIR/web"
PORT="${1:-8080}"

if [[ ! -f "$WEB_DIR/tower_defense.js" ]]; then
  echo "ERROR: web/tower_defense.js not found — run ./build.sh first." >&2
  exit 1
fi

echo "==> Serving $WEB_DIR on http://localhost:$PORT"
echo "    Press Ctrl+C to stop."
exec python3 -m http.server "$PORT" --directory "$WEB_DIR"
