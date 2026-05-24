# gdphysx

A lightweight OpenGL-based graphics and physics engine built with C++20, GLFW, and GLM. Features a modular architecture with separate physics and rendering systems.

## About Copilot Usage

**GitHub Copilot was used ONLY for convenience purposes:**
- Documentation generation (`.github/copilot-instructions.md`)
- Workflow automation setup (`.github/workflows/copilot-setup-steps.yml`)
- Commit message composition
- README and setup file creation

**Copilot was NOT used for any coding content.** All core implementation (`src/`, physics engine, shader system, rendering pipeline) is original code.

## Features

- **Physics Engine**: Particle-based physics simulation with kinematic motion
  - Position, velocity, and acceleration updates
  - Configurable delta-time integration
  
- **Rendering System**: Modern OpenGL 4.6 core profile
  - Shader compilation and linking utilities
  - GLFW window management
  - GLM matrix transformations

- **Cross-Platform**: Builds on Linux and Windows (MinGW)
  - Deterministic CMake-based build
  - Automatic dependency fetching

## Quick Start

### Prerequisites
- CMake 3.10+
- C++20 compiler (g++, clang, or MSVC)
- OpenGL development libraries

### Build

**Linux:**
```bash
cmake -B build-linux && cmake --build build-linux
./build-linux/gdphysx
```

**Windows (MinGW):**
```bash
cmake -B build-windows -G "MinGW Makefiles" && cmake --build build-windows
./build-windows/gdphysx.exe
```

### Clean Rebuild
```bash
rm -rf build-linux build-windows && cmake -B build-linux && cmake --build build-linux
```

## Project Structure

```
gdphysx/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── shader.cpp            # Shader compilation system
│   ├── particle.cpp          # Physics particle engine
│   ├── headers/
│   │   ├── main.h
│   │   ├── shader.h
│   │   └── particle.h
│   └── shaders/
│       ├── sample.vert       # Vertex shader
│       └── sample.frag       # Fragment shader
├── CMakeLists.txt            # Build configuration
├── .github/
│   ├── copilot-instructions.md       # Copilot setup guide
│   └── workflows/
│       └── copilot-setup-steps.yml   # Automated CI/CD setup
└── thirdparty/               # Auto-fetched dependencies (GLFW, Glad, GLM)
```

## Architecture

### Physics Engine (`melonyx` namespace)
- **Particle Class**: Represents physical objects with position, velocity, and acceleration
- **Kinematic Equations**: 
  - Position: `P = P₀ + V*t + 0.5*A*t²`
  - Velocity: `V = V₀ + A*t`
- Separate update methods for clarity and numerical stability

### Rendering System
- **Shader Compilation**: Loads and compiles `.vert` and `.frag` shader files
- **OpenGL Context**: Glad GL 4.6 core profile with GLFW window management
- **Orthographic Camera**: 2D projection with matrix transformations

### Main Loop
- 800×800 square window with centered title
- Delta-time based physics updates
- Render pipeline with shader binding and draw calls

## Dependencies

All dependencies are automatically fetched via CMake FetchContent:
- **GLFW 3.4**: Window and input management
- **Glad 2.0.6**: OpenGL 4.6 function loader
- **GLM 1.0.1**: Vector and matrix mathematics

## Code Conventions

- **Namespace**: Physics code uses `namespace melonyx`
- **Naming**: PascalCase for classes/members, camelCase for functions
- **Documentation**: JSDoc-style headers with `@file` and `@author` tags
- **Headers**: `.h` extension with pragma once guards
- **Math**: GLM types for all vector/matrix operations

## Development

For detailed build instructions, conventions, and architecture notes, see `.github/copilot-instructions.md`.

### Running Tests
Currently no unit tests. Validation is done through visual rendering.

### IDE Integration
- VSCode: Uses CMake Tools extension with Unix Makefiles generator
- Compile commands exported via `CMAKE_EXPORT_COMPILE_COMMANDS`
- LSP integration enabled for code intelligence

## Author
Erica Mauriz Barundia
