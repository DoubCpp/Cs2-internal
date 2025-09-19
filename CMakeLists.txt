cmake_minimum_required(VERSION 3.20)
project(Internal-Menu-Cs2)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compile options
if(MSVC)
    add_compile_options(/W3 /O2 /MT)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -O2)
endif()

# Dependencies directories
set(DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
set(IMGUI_DIR "${DEPENDENCIES_DIR}/imgui")
set(HOOK_DIR "${DEPENDENCIES_DIR}/hook")
set(MINHOOK_DIR "${DEPENDENCIES_DIR}/minhook")

# ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
)

# Hook source files
set(HOOK_SOURCES
    ${HOOK_DIR}/hook.cpp
)

# MinHook source files
set(MINHOOK_SOURCES
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde32.c
    ${MINHOOK_DIR}/src/hde64.c
)

# Main DLL source files (including all new systems)
set(SOURCES
    src/dllmain.cpp
    src/gui/menu.cpp
    src/core/memory.cpp
    src/core/math.cpp
    src/game/pattern.cpp
    # Visual features
    src/features/visuals/visuals.cpp
    src/features/visuals/esp.cpp
    # Legit features
    src/features/legit/legit.cpp
    src/features/legit/aimbot.cpp
    src/features/legit/triggerbot.cpp
    ${IMGUI_SOURCES}
    ${HOOK_SOURCES}
    ${MINHOOK_SOURCES}
)

# Create DLL
add_library(Internal-Menu-Cs2 SHARED ${SOURCES})

# Include directories
target_include_directories(Internal-Menu-Cs2 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${DEPENDENCIES_DIR}
    ${IMGUI_DIR}
    ${HOOK_DIR}
    ${MINHOOK_DIR}
)

# Link libraries
target_link_libraries(Internal-Menu-Cs2 PRIVATE 
    d3d11
    dxgi
    d3dcompiler
)

# Set output directory
set_target_properties(Internal-Menu-Cs2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release"
    OUTPUT_NAME "Internal-Menu-Cs2"
)

# Add injector subdirectory
add_subdirectory(injector)