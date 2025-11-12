cmake_minimum_required(VERSION 3.16)
project(Modularity LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===== Compiler flags =====
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# ===== Include project headers =====
include_directories(${PROJECT_SOURCE_DIR}/include)

# ===== GLAD library =====
add_library(glad STATIC src/ThirdParty/glad/glad.c)
target_include_directories(glad PUBLIC src/ThirdParty/glad)

# ===== GLM (header-only) =====
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE src/ThirdParty/glm)

# ===== Collect all source files =====
file(GLOB_RECURSE PROJECT_SOURCES
    src/*.cpp
    src/*.c
)

# Exclude third-party sources (glad, etc.)
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "src/ThirdParty/")

# Optional: Exclude main.cpp from the core library if it exists in src/
list(FILTER PROJECT_SOURCES EXCLUDE REGEX "src/main\\.cpp")

# ===== Core library =====
add_library(core ${PROJECT_SOURCES})
target_include_directories(core PUBLIC include)
target_link_libraries(core PRIVATE glad glm)

# ===== GLFW setup =====
if(WIN32)
    find_package(glfw3 REQUIRED)
    set(PLATFORM_LIBS glfw3 opengl32 gdi32)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)
    set(PLATFORM_LIBS ${GLFW_LIBRARIES} GL dl pthread)
    include_directories(${GLFW_INCLUDE_DIRS})
endif()

# ===== Main executable =====
add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE core glad glm ${PLATFORM_LIBS})

# ===== Optional: run target =====
add_custom_target(run
    COMMAND main
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# ===== Clean all build files =====
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files..."
)
