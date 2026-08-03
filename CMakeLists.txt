cmake_minimum_required(VERSION 3.14)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

project(melonyx VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------------------
# Dependencies via FetchContent
# ------------------------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty" CACHE PATH "Base Directory of libs" FORCE)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.6
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(glad)

glad_add_library(glad_gl_core_46 STATIC
    REPRODUCIBLE
    LOADER
    API gl:core=4.6
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# OpenGL System Library
find_package(OpenGL REQUIRED)

# ------------------------------------------------------------------------------
# Executable Definition & Existing Source Files
# ------------------------------------------------------------------------------
set(SOURCES
    # Core Engine & Entry Point
    src/main.cpp
    src/shader.cpp
    src/particle.cpp
    src/render_particle.cpp
    src/world_particle.cpp
    src/tiny_obj_loader.cpp

    # Physics Core & Constraints
    src/particlecontact.cpp
    src/contactresolver.cpp
    src/particlelink.cpp
    src/rod.cpp
    src/cable.cpp

    # Forces & Springs
    src/force/gravity_force_gen.cpp
    #src/force/drag_force_gen.cpp
    src/force/force_registry.cpp

    # Engine Headers (for Visual Studio / IDE project tree)
    src/headers/main.h
    src/headers/shader.h
    src/headers/render_particle.h
    src/headers/world_particle.h
    src/headers/OpenGLObject.h
    src/headers/particlecontact.h
    src/headers/contactresolver.h
    src/headers/cable.h
    
    src/headers/force_gen.h
    src/headers/drag_force_gen.h
    src/headers/force_registry.h
    

    # Pirate Logic Headers (Header-Only)
    src/headers/pirate/constants.h
    src/headers/pirate/game_state.h
    src/headers/pirate/level.h
    src/headers/pirate/bottle.h
    src/headers/pirate/collision.h
    src/headers/pirate/cannon.h
    src/headers/pirate/pirate.h
)

add_executable(melonyx ${SOURCES} "src/headers/pirate/bottle_placer.h" "src/bottle_placer.cpp" "src/drag_force_gen.cpp" "src/headers/wind_force_gen.h" "src/wind_force_gen.cpp")

# Allows `#include "headers/..."` and `#include "headers/pirate/..."`
target_include_directories(melonyx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
)

# Math macros for MSVC / MinGW
target_compile_definitions(melonyx PRIVATE 
    _USE_MATH_DEFINES 
    NOMINMAX
)

# MSVC Compiler Options
if(MSVC)
    target_compile_options(melonyx PRIVATE /permissive- /W3 /D_CRT_SECURE_NO_WARNINGS)
endif()

# ------------------------------------------------------------------------------
# Link Libraries & Platform Options
# ------------------------------------------------------------------------------
target_link_libraries(melonyx PRIVATE
    glfw
    glad_gl_core_46
    OpenGL::GL
    glm::glm
)

# Windows Specific Linkage
if(WIN32)
    target_link_libraries(melonyx PRIVATE opengl32)
    if(MINGW)
        set_target_properties(melonyx PROPERTIES
            LINK_FLAGS "-static -static-libgcc -static-libstdc++"
        )
    endif()
endif()

# Linux Specific Linkage
if(UNIX AND NOT APPLE)
    target_link_libraries(melonyx PRIVATE ${CMAKE_DL_LIBS} pthread)
endif()

# ------------------------------------------------------------------------------
# Post-Build Asset Copies
# ------------------------------------------------------------------------------
add_custom_command(TARGET melonyx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
        "$<TARGET_FILE_DIR:melonyx>/shaders"
    COMMENT "Copying shaders..."
)

add_custom_command(TARGET melonyx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/3D"
        "$<TARGET_FILE_DIR:melonyx>/3D"
    COMMENT "Copying 3D assets..."
)