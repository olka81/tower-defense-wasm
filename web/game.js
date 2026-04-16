/**
 * game.js — Canvas renderer + game loop
 *
 * The C++ core (tower_defense.wasm) exposes six functions via Emscripten
 * bindings. This file owns everything visual and the RAF loop.
 *
 *   Module.game_init(w, h)          — create game world
 *   Module.game_update(dt)          — advance simulation
 *   Module.game_left_click(x, y)    — place tower
 *   Module.game_right_click(x, y)   — toggle tower type
 *   Module.game_render_data()       — JSON snapshot → draw
 *   Module.game_state()             — JSON HUD data
 */

'use strict';

const CANVAS_W = 768;
const CANVAS_H = 576;

// Tile-type constants must match the C++ Tile enum
const Tile = { Empty: 0, Path: 1, Tower: 2 };

// Palette
const COLOR = {
  bg:          '#1e1e1e',
  tileEmpty:   '#3a6b3a',
  tilePath:    '#b49650',
  tileTower:   '#3c3c3c',
  towerBasic:  '#64b4c8',
  towerSniper: '#5050a0',
  enemyBasic:  '#dc3232',
  enemyFast:   '#ff8c00',
  hpBg:        '#3c3c3c',
  hpFg:        '#00dc00',
  projectile:  '#ffe632',
  rangeCircle: 'rgba(255,255,255,0.08)',
};

// ---- DOM refs --------------------------------------------------------------
const canvas   = document.getElementById('canvas');
const ctx      = canvas.getContext('2d');
const elStatus = document.getElementById('status');
const elLives  = document.getElementById('hud-lives');
const elGold   = document.getElementById('hud-gold');
const elWave   = document.getElementById('hud-wave');
const elSel    = document.getElementById('hud-selected');

// ---- Boot ------------------------------------------------------------------
TowerDefense().then(Module => {
  elStatus.textContent = 'Running';

  Module.game_init(CANVAS_W, CANVAS_H);

  // Input
  canvas.addEventListener('click', e => {
    const r = canvas.getBoundingClientRect();
    Module.game_left_click(
      Math.round(e.clientX - r.left),
      Math.round(e.clientY - r.top)
    );
  });

  canvas.addEventListener('contextmenu', e => {
    e.preventDefault();
    const r = canvas.getBoundingClientRect();
    Module.game_right_click(
      Math.round(e.clientX - r.left),
      Math.round(e.clientY - r.top)
    );
  });

  // Game loop
  let lastTime = null;

  function loop(timestamp) {
    if (lastTime === null) lastTime = timestamp;
    const dt = Math.min((timestamp - lastTime) / 1000, 0.05);
    lastTime = timestamp;

    Module.game_update(dt);

    const renderData = JSON.parse(Module.game_render_data());
    const state      = JSON.parse(Module.game_state());

    draw(renderData);
    updateHud(state);

    if (!state.gameOver) {
      requestAnimationFrame(loop);
    } else {
      drawGameOver(ctx);
    }
  }

  requestAnimationFrame(loop);
}).catch(err => {
  elStatus.textContent = 'Failed to load WASM: ' + err;
  console.error(err);
});

// ---- HUD -------------------------------------------------------------------
function updateHud(state) {
  elLives.textContent = '❤ ' + state.lives;
  elGold.textContent  = 'Gold: ' + state.gold;
  elWave.textContent  = 'Wave: ' + state.wave;
  elSel.textContent   = state.selected === 'Sniper'
    ? 'Tower: Sniper ($100)'
    : 'Tower: Basic ($50)';
}

// ---- Rendering -------------------------------------------------------------
function draw(d) {
  ctx.clearRect(0, 0, CANVAS_W, CANVAS_H);

  drawTiles(d);
  drawTowers(d.towers);
  drawEnemies(d.enemies);
  drawProjectiles(d.projectiles);
}

function drawTiles(d) {
  const { cols, rows, tileSize, tiles } = d;
  for (let row = 0; row < rows; row++) {
    for (let col = 0; col < cols; col++) {
      const t = tiles[row * cols + col];
      ctx.fillStyle =
        t === Tile.Path  ? COLOR.tilePath  :
        t === Tile.Tower ? COLOR.tileTower :
                           COLOR.tileEmpty;
      ctx.fillRect(col * tileSize, row * tileSize, tileSize - 1, tileSize - 1);
    }
  }
}

function drawTowers(towers) {
  for (const t of towers) {
    // Range indicator
    ctx.beginPath();
    ctx.arc(t.x, t.y, t.range, 0, Math.PI * 2);
    ctx.fillStyle = COLOR.rangeCircle;
    ctx.fill();

    // Tower body
    ctx.fillStyle = t.type === 1 ? COLOR.towerSniper : COLOR.towerBasic;
    ctx.fillRect(t.x - 14, t.y - 14, 28, 28);

    // Barrel
    ctx.strokeStyle = '#ffffff66';
    ctx.lineWidth = 3;
    ctx.beginPath();
    ctx.moveTo(t.x, t.y);
    ctx.lineTo(t.x, t.y - 18);
    ctx.stroke();
  }
}

function drawEnemies(enemies) {
  for (const e of enemies) {
    ctx.fillStyle = e.type === 1 ? COLOR.enemyFast : COLOR.enemyBasic;
    ctx.beginPath();
    ctx.arc(e.x, e.y, 10, 0, Math.PI * 2);
    ctx.fill();

    // HP bar
    const barW = 20, barH = 4;
    const bx = e.x - barW / 2, by = e.y - 18;
    ctx.fillStyle = COLOR.hpBg;
    ctx.fillRect(bx, by, barW, barH);
    ctx.fillStyle = COLOR.hpFg;
    ctx.fillRect(bx, by, barW * (e.hp / e.maxHp), barH);
  }
}

function drawProjectiles(projectiles) {
  ctx.fillStyle = COLOR.projectile;
  for (const p of projectiles) {
    ctx.beginPath();
    ctx.arc(p.x, p.y, 4, 0, Math.PI * 2);
    ctx.fill();
  }
}

function drawGameOver(ctx) {
  ctx.fillStyle = 'rgba(0,0,0,0.65)';
  ctx.fillRect(0, 0, CANVAS_W, CANVAS_H);
  ctx.fillStyle = '#ff4444';
  ctx.font = 'bold 56px monospace';
  ctx.textAlign = 'center';
  ctx.fillText('GAME OVER', CANVAS_W / 2, CANVAS_H / 2);
  ctx.fillStyle = '#aaaaaa';
  ctx.font = '22px monospace';
  ctx.fillText('Refresh to play again', CANVAS_W / 2, CANVAS_H / 2 + 50);
}
