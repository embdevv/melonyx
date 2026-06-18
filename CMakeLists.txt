cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

project(melonyx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
    CACHE PATH "Base Directory of libs"
    FORCE
)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ------------------------------------------------------------
# GLFW
# ------------------------------------------------------------

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# ------------------------------------------------------------
# GLAD
# ------------------------------------------------------------

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
    SOURCE_SUBDIR cmake
)

FetchContent_MakeAvailable(glad)

glad_add_library(
    glad_gl_core_46
    STATIC
    REPRODUCIBLE
    LOADER
    API gl:core=4.6
)

# ------------------------------------------------------------
# GLM
# ------------------------------------------------------------

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)

# ------------------------------------------------------------
# OpenGL
# ------------------------------------------------------------

find_package(OpenGL REQUIRED)

add_compile_definitions(_USE_MATH_DEFINES)

# ------------------------------------------------------------
# Executable
# ------------------------------------------------------------

add_executable(melonyx
    src/main.cpp
    src/shader.cpp
    src/particle.cpp
    src/render_particle.cpp
    src/world_particle.cpp
    src/tiny_obj_loader.cpp

    src/contactresolver.cpp
    src/particlecontact.cpp
    src/particlelink.cpp
    src/rod.cpp
    src/cable.cpp

    src/spring/particlespring.cpp
    src/spring/anchoredspring.cpp

    src/force/gravity_force_gen.cpp
    src/force/drag_force_gen.cpp
    src/force/force_registry.cpp

    src/headers/OpenGLObject.h
    src/headers/shader.h
    src/headers/render_particle.h
    src/headers/world_particle.h
    src/headers/tiny_obj_loader.h

    src/headers/contactresolver.h
    src/headers/particlecontact.h
    src/headers/particlelink.h
    src/headers/rod.h
    src/headers/cable.h

    src/headers/force_gen.h
    src/headers/gravity_force_gen.h
    src/headers/drag_force_gen.h
    src/headers/force_registry.h

    src/headers/particlespring.h
    src/headers/anchoredspring.h

    src/headers/particle.h
)

# ------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------

target_include_directories(melonyx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
)

# ------------------------------------------------------------
# Link Libraries
# ------------------------------------------------------------

target_link_libraries(melonyx PRIVATE
    glfw
    glad_gl_core_46
    OpenGL::GL
    glm::glm
)

# ------------------------------------------------------------
# Linux
# ------------------------------------------------------------

if(UNIX AND NOT APPLE)
    target_link_libraries(melonyx PRIVATE
        dl
        pthread
    )
endif()

# ------------------------------------------------------------
# Windows
# ------------------------------------------------------------

if(WIN32)
    target_link_libraries(melonyx PRIVATE
        opengl32
    )
endif()

# ------------------------------------------------------------
# Copy Shaders
# ------------------------------------------------------------

add_custom_command(TARGET melonyx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
        "$<TARGET_FILE_DIR:melonyx>/shaders"
    COMMENT "Copying shaders"
)

# ------------------------------------------------------------
# Copy 3D Assets
# ------------------------------------------------------------

add_custom_command(TARGET melonyx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/3D"
        "$<TARGET_FILE_DIR:melonyx>/3D"
    COMMENT "Copying 3D assets"
)