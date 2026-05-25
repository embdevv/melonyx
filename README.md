# melonyx

A lightweight OpenGL-based graphics and physics engine built with C++20, GLFW, and GLM. Features a modular architecture with separate physics and rendering systems.

## AI Disclosure

This entire README was written by Copilot, then reviewed by me. The project was made inside CachyOS, an Arch Linux-based distribution. It was written in VSCodium and used the Copilot CLI (https://github.com/features/copilot/cli) package to automate descriptions for this project. AI model used inside the project was provided by Claude, specifically the Haiku model.

**GitHub Copilot was used for efficiency in non-coding tasks only:**

### ✅ What Copilot Was Used For:
- Documentation generation (`.github/copilot-instructions.md`)
- GitHub Actions workflow automation (`.github/workflows/copilot-setup-steps.yml`)
- Commit message composition and formatting
- README and supporting documentation files

### ❌ What Copilot Was NOT Used For:
- **Core implementation** - All physics engine code is original
- **Rendering system** - All shader compilation and OpenGL code is original
- **Main application loop** - All business logic is original
- **Project architecture** - All design decisions and system design are original

**Transparency Statement:** Copilot was leveraged as a productivity tool for documentation, automation, and communication artifacts. The actual software engineering work—designing the physics system, implementing the rendering pipeline, architecting the particle system, and all algorithmic decisions—was done without AI assistance. This approach allowed focus on core development while automating routine documentation and setup tasks.

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
./build-linux/melonyx
```

**Windows (MinGW):**
```bash
cmake -B build-windows -G "MinGW Makefiles" && cmake --build build-windows
./build-windows/melonyx.exe
```

### Clean Rebuild
```bash
rm -rf build-linux build-windows && cmake -B build-linux && cmake --build build-linux
```

## Project Structure

```
melonyx/
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
