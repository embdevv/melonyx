#pragma once

enum class GameState {
    AIMING,     // Player rotating cannon, setting power
    LAUNCHED,   // Pirate in flight, physics running
    RESULT,     // Flight ended, tallying bottles
    GAME_OVER,  // Failed quota or drowned with 0 hits
    WIN         // All 3 levels cleared with quota met
};