cmake_minimum_required(VERSION 3.16)
project(Modularity LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include your project headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# ===== GLAD library =====
add_library(glad STATIC src/ThirdParty/glad/glad.c)
target_include_directories(glad PUBLIC src/ThirdParty/glad)

# ===== Core library (your code) =====
file(GLOB_RECURSE CORE_SOURCES src/WinView/*.cpp)
add_library(core ${CORE_SOURCES})
target_include_directories(core PUBLIC include)
target_link_libraries(core PRIVATE glad)

# ===== Find GLFW =====
if(WIN32)
    # Assumes GLFW is installed manually on Windows
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
target_link_libraries(main PRIVATE core glad ${PLATFORM_LIBS})

# ===== Optional: run target =====
add_custom_target(run
    COMMAND main
    DEPENDS main
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build files..."
)
