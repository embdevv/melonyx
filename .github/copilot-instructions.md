# Copilot Instructions for gdphysx

## Build & Compilation

**Prerequisites**: CMake 3.10+, C++20 compiler (g++, clang, or MSVC)

### Build Commands

**Linux**:
```bash
cmake -B build-linux && cmake --build build-linux
./build-linux/gdphysx
```

**Windows (MinGW)**:
```bash
cmake -B build-windows -G "MinGW Makefiles" && cmake --build build-windows
./build-windows/gdphysx.exe
```

**Clean rebuild**:
```bash
rm -rf build-linux build-windows && cmake -B build-linux && cmake --build build-linux
```

### Build System Details
- CMake uses FetchContent to automatically fetch dependencies (GLFW, Glad, GLM, OpenGL)
- Dependencies are cached in `thirdparty/` directory
- `compile_commands.json` is generated and linked from project root for IDE integration
- Shaders are automatically copied to the executable directory post-build via custom CMake command

## High-Level Architecture

**gdphysx** is a lightweight OpenGL-based graphics and physics engine using the `melonyx` namespace.

### Core Components

1. **Rendering System** (`src/shader.cpp`, `src/headers/shader.h`)
   - Shader compilation and linking (GL 4.6 core profile)
   - Compiles `.vert` and `.frag` files from `src/shaders/`
   - Returns GLuint shader program IDs

2. **Physics Engine** (`src/particle.cpp`, `src/headers/particle.h`)
   - Particle class with position, velocity, acceleration (glm::vec3)
   - Uses kinematic equations: P = P₀ + V*t + 0.5*A*t²
   - Update methods separate position/velocity calculations for numerical stability

3. **Main Loop** (`src/main.cpp`)
   - GLFW window initialization (800x800 default)
   - Orthographic camera setup
   - Render loop with delta time calculations
   - Centered dark red sphere rendering

### Dependency Graph
```
gdphysx (executable)
├── glfw (window/input management)
├── glad_gl_core_46 (OpenGL function loader)
├── OpenGL::GL (graphics API)
└── glm::glm (vector/matrix math)
```

## Key Conventions

### Namespace
- All physics and simulation code uses `namespace melonyx`
- Graphics utilities (shaders, window management) are in global scope

### Code Organization
- **Headers**: `src/headers/*.h` (`.h` extension, pragma once guards)
- **Implementation**: `src/*.cpp`
- **Shaders**: `src/shaders/*.vert`, `src/shaders/*.frag`
- **Third-party**: `thirdparty/` (auto-fetched by CMake, do not edit)

### Naming Conventions
- **Classes**: PascalCase (e.g., `Particle`, `Shader`)
- **Member variables**: PascalCase (e.g., `Position`, `Velocity`)
- **Constants**: UPPER_CASE (e.g., `WINDOW_SIZE`, `WINDOW_TITLE`)
- **Functions**: camelCase (e.g., `UpdatePosition()`, `compileShaders()`)
- **Namespaces**: lowercase (e.g., `melonyx`)

### Math Library
- Use GLM types for all vector/matrix operations: `glm::vec3`, `glm::mat4`, etc.
- Include appropriate GLM headers: `<glm/gtc/matrix_transform.hpp>`, `<glm/gtc/type_ptr.hpp>`
- GLM column-major matrices by default (OpenGL convention)

### Documentation
- Use JSDoc-style comment blocks: `/** @file description @author name */`
- Include `@file` with brief description and `@author` tags in source files
- Document physics formulas inline (e.g., `// Vf = Vi + a * t`)

### Physics Implementation
- Position/velocity updates use separate methods for clarity:
  - `UpdatePosition(dt)`: Kinematic position update
  - `UpdateVelocity(dt)`: Velocity update from acceleration
  - `Update(dt)`: Combined or simplified update
- Delta time (`dt`) is always in seconds; use `float` precision for consistency

### GLFW/OpenGL Integration
- Use Glad for OpenGL function loading, not raw GL bindings
- GLFW handles window creation and event polling
- Window is square (800x800) with centered title
- Orthographic projection for 2D rendering

## CMakeLists.txt Structure
- Uses FetchContent for reproducible builds across platforms
- Includes platform-specific linking:
  - Linux: Link `dl` and `pthread`
  - Windows MinGW: Static linking with `-static -static-libgcc -static-libstdc++`
- Post-build shader copy ensures runtime availability
- Export compile commands for IDE/LSP tooling

## Development Tips
- Check `compile_commands.json` is linked to your build directory for IDE autocompletion
- Run from project root: executable finds shaders in relative `shaders/` directory
- Use `.vscode/settings.json` for CMake integration (Unix Makefiles generator on Linux)
- VSCode uses `ms-vscode.cmake-tools` extension for build integration
