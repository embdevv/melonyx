cmake_minimum_required(VERSION 3.10)

if (POLICY CMP0141)
    cmake_policy(SET CMP0072 NEW)
endif()

project("melonyx" VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages (Nix provides these via buildInputs)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
#find_package(glm REQUIRED)

# Executable
add_executable(melonyx
    src/main.cpp
    src/shader.cpp
    src/particle.cpp
    src/render_particle.cpp
    src/world_particle.cpp
    src/tiny_obj_loader.cpp
    src/contactresolver.cpp
    src/spring/particlespring.cpp
    src/particlecontact.cpp
    src/particlelink.cpp
    src/cable.cpp
    src/rod.cpp
    src/force/gravity_force_gen.cpp
    src/force/drag_force_gen.cpp
    src/force/force_registry.cpp
    src/spring/anchoredspring.cpp
    src/glad.c  # Ensure this path is correct
)

# Include paths
target_include_directories(melonyx PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm
)

# Link libraries
target_link_libraries(melonyx PRIVATE
    glfw
    #glm::glm
    OpenGL::GL
)

# Platform specific
if(WIN32)
    target_link_libraries(melonyx PRIVATE opengl32)
    set_target_properties(melonyx PROPERTIES
        LINK_FLAGS "-static -static-libgcc -static-libstdc++"
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(melonyx PRIVATE dl pthread)
endif()

# Post-build: Copy resources
add_custom_command(TARGET melonyx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders"
        "$<TARGET_FILE_DIR:melonyx>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/3D"
        "$<TARGET_FILE_DIR:melonyx>/3D"
    COMMENT "Copying shaders and assets"
)