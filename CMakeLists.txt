cmake_minimum_required(VERSION 3.10)
project(midi2gcode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add FetchContent for downloading dependencies
include(FetchContent)

# Download and configure GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Download Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

# Download JSON library
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Download GLM for 3D math
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Download PortMidi for MIDI playback
FetchContent_Declare(
    portmidi
    GIT_REPOSITORY https://github.com/PortMidi/portmidi.git
    GIT_TAG v2.0.4
)
set(PORTMIDI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PORTMIDI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PORTMIDI_BUILD_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
set(PORTMIDI_BUILD_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(portmidi)

# Download GLAD
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
FetchContent_MakeAvailable(glad)

# Create ImGui library
add_library(imgui_impl
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_impl PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(imgui_impl PUBLIC
    glfw
    ${OPENGL_LIBRARIES}
)

# Main executable
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/midi_parser.cpp
    src/gcode_generator.cpp
    src/app_settings.cpp
    src/gcode_visualizer.cpp
    src/midi_player.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glm_SOURCE_DIR}
    ${portmidi_SOURCE_DIR}/pm_common
    ${portmidi_SOURCE_DIR}/porttime
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui_impl
    glfw
    nlohmann_json::nlohmann_json
    glm::glm
    glad
    portmidi
    ${OPENGL_LIBRARIES}
)
