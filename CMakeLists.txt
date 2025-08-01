cmake_minimum_required(VERSION 3.10)
project(Cyclone3D C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Add include directories for headers
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
)

# Explicitly list all your .c source files.
set(SOURCES
    cyclone.c
    utils.c
    gameInit.c
    player.c
    object.c
    platforms.c
    gamestate.c
    vector.c
    renderer.c
)

# Create the executable from the source files
add_executable(cyclone ${SOURCES})

# Link the required libraries to your executable.
target_link_libraries(cyclone
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    m
)

# --- NEW SECTION TO COPY LEVEL FILE ---
# This command will run after the 'gravitydash' executable is built.
# It copies the level file from the source directory to the executable's directory.

add_custom_command(
    TARGET cyclone POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/*.glsl"
    "$<TARGET_FILE_DIR:cyclone>/"
    COMMENT "Copying shaders..."
)
