cmake_minimum_required(VERSION 3.16)
project(Modularity LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== WINDOWS FIXES (only active on Windows) ====================
if(WIN32)
    add_compile_definitions(
        NOMINMAX                  # Fixes std::min/std::max clash with Windows.h
        WIN32_LEAN_AND_MEAN       # Speeds up Windows.h includes
        _CRT_SECURE_NO_WARNINGS   # Silences strncpy, sscanf, etc. warnings
    )
endif()

# ==================== Compiler flags ====================
if(MSVC)
    add_compile_options(/W4 /O2 /permissive- /MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

# ==================== Third-party libraries ====================

add_subdirectory(src/ThirdParty/glfw EXCLUDE_FROM_ALL)
find_package(OpenGL REQUIRED)

# GLAD
add_library(glad STATIC src/ThirdParty/glad/glad.c)
target_include_directories(glad PUBLIC src/ThirdParty/glad)

# GLM (header-only)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE src/ThirdParty/glm)

# ImGuizmo ←←← THIS WAS THE BUG
add_library(imguizmo STATIC
    src/ThirdParty/ImGuizmo/ImGuizmo.cpp
)
target_include_directories(imguizmo PUBLIC src/ThirdParty/ImGuizmo)
target_link_libraries(imguizmo PUBLIC imgui glm)   # ImGuizmo uses ImGui + GLM

# Dear ImGui
set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/src/ThirdParty/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PRIVATE glfw)

# ==================== Your code ====================
file(GLOB_RECURSE PROJECT_SOURCES
    src/*.cpp
    src/*.c
)
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "src/ThirdParty/|src/main\\.cpp")

add_library(core STATIC ${PROJECT_SOURCES})
target_include_directories(core PUBLIC include)
target_link_libraries(core PUBLIC glad glm imgui imguizmo)

# ==================== Executable ====================
add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE core glfw OpenGL::GL)

# Optional: remove console window on Windows (uncomment if you want GUI-only app)
#if(WIN32)
#    set_target_properties(main PROPERTIES WIN32_EXECUTABLE TRUE)
#endif()

# Linux needs these extra libs
if(NOT WIN32)
    target_link_libraries(main PRIVATE dl pthread X11)
endif()

# ==================== Copy Resources folder after build ====================
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Resources
        $<TARGET_FILE_DIR:main>/Resources
)