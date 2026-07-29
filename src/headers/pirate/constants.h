#pragma once

// ── World ──────────────────────────────────────────────────────────────────
constexpr float ORTHO_SIZE = 450.0f;   // +-450 units in x and y
constexpr float WATER_LINE = -280.0f;  // y below this = drown

// ── Physics ────────────────────────────────────────────────────────────────
constexpr float GRAVITY = 300.0f;   // world units / s^2
constexpr float DRAG_K1 = 0.02f;
constexpr float DRAG_K2 = 0.01f;

// ── Cannon ─────────────────────────────────────────────────────────────────
constexpr float MIN_ANGLE = -10.0f;   // degrees from horizontal
constexpr float MAX_ANGLE = 80.0f;
constexpr float MIN_POWER = 100.0f;   // world-units / s
constexpr float MAX_POWER = 600.0f;
constexpr float POWER_STEP = 5.0f;   // per keypress
constexpr float ANGLE_STEP = 2.0f;   // degrees per keypress

// ── Pirate ─────────────────────────────────────────────────────────────────
constexpr float PIRATE_RADIUS = 20.0f;
constexpr float SPIN_FACTOR = 0.05f;  // launch spin multiplier
constexpr float SPIN_MULTIPLIER = 2.0f;   // collision spin multiplier
constexpr float RESTITUTION = 0.4f;   // bounce factor on obstacles

// ── Bottles ────────────────────────────────────────────────────────────────
constexpr float BOTTLE_RADIUS = 12.0f;
constexpr float CHAIN_RADIUS = 60.0f;   // chain-reaction trigger radius

// ── Trajectory preview ─────────────────────────────────────────────────────
constexpr int   PREVIEW_STEPS = 60;
constexpr float PREVIEW_DT = 0.05f;

// ── Quota (cumulative across all 3 levels) ─────────────────────────────────
constexpr int   TOTAL_QUOTA = 9;