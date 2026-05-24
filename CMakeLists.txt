cmake_minimum_required(VERSION 3.10)

if (POLICY CMP0141)
    cmake_policy(SET CMP0072 NEW)
endif()

project("melonyx")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty CACHE PATH "Base Directory of libs" FORCE)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
    SOURCE_SUBDIR cmake
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
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# OpenGL
find_package(OpenGL REQUIRED)

# Executable
add_executable(melonyx
    src/main.cpp
    src/headers/shader.h
    src/shader.cpp
    src/particle.cpp
    src/headers/render_particle.h
    src/render_particle.cpp
    src/headers/OpenGLObject.h
)

# Include headers folder
target_include_directories(melonyx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
)

# Link libraries
target_link_libraries(melonyx
    glfw
    OpenGL::GL
    glad_gl_core_46
    glm::glm
)

# Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(melonyx dl pthread)
endif()

# Windows MinGW static
if(WIN32)
    target_link_libraries(melonyx opengl32)
    set_target_properties(melonyx PROPERTIES
        LINK_FLAGS "-static -static-libgcc -static-libstdc++"
    )
endif()

# Copy shaders
add_custom_command(TARGET melonyx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
        "$<TARGET_FILE_DIR:melonyx>/shaders"
    COMMENT "Copying shaders"
)

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET melonyx PROPERTY CXX_STANDARD 20)
endif()