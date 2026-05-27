# Programming Challenge 1 - Particle Race Simulation

A physics-based simulation of 4 particles racing from the corners of a 700x700 screen to the center (0, 0, 0) with constant acceleration.

## Particle Details

| Particle | Start Position | Velocity | Acceleration |
|----------|---|---|---|
| **Red** | (-350, -350, 201) | 80 m/s | 14.5 m/s² |
| **Green** | (350, -350, 173) | 90 m/s | 8 m/s² |
| **Blue** | (350, 350, -300) | 130 m/s | 1 m/s² |
| **Yellow** | (-350, 350, -150) | 110 m/s | 3 m/s² |

## Results

Using kinematic equations with constant acceleration:
- **s = v₀t + 0.5at²**
- **v = v₀ + at**

| Rank | Particle | Time | Final Velocity | Avg Velocity |
|------|----------|------|---|---|
| 🥇 1 | Blue | 4.38s | 134.38 m/s | 132.19 m/s |
| 🥈 2 | Yellow | 4.43s | 123.30 m/s | 116.65 m/s |
| 🥉 3 | Red | 4.69s | 147.96 m/s | 113.98 m/s |
| 4 | Green | 4.80s | 128.41 m/s | 109.21 m/s |

## Building

### Linux/macOS

```bash
# Using the build script (recommended)
chmod +x build-race.sh
./build-race.sh

# Or manually
mkdir -p build-race && cd build-race
cmake ..
cmake --build . --target race_simulation
./race_simulation
```

### Windows (with CMake and MinGW/MSVC)

```cmd
mkdir build-race
cd build-race
cmake .. -G "MinGW Makefiles"
cmake --build . --target race_simulation
./race_simulation.exe
```

## Implementation Details

- **Language**: C++ (C++20)
- **Math Library**: GLM (header-only)
- **Physics**: Constant acceleration kinematics
- **Precision**: All values rounded to hundredths place
- **Cross-platform**: Builds on Windows, Linux, and macOS

## Files

- `src/race_particle.h` - RaceParticle class definition
- `src/race_particle.cpp` - Physics calculations and results
- `src/race_main.cpp` - Main simulation and display logic
- `CMakeLists.txt` - Build configuration (updated)
