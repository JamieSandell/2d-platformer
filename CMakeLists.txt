cmake_minimum_required(VERSION 3.20)
project(2d_platformer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Default to Debug when no build type is given (single-config generators only)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Platform independent code
add_executable(2d_platformer
        src/game.c
        headers/platform.h
        headers/game.h
        headers/colour.h
)
target_include_directories(2d_platformer PRIVATE headers)
target_compile_options(2d_platformer PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion -Wsign-conversion
        -Wdouble-promotion
        -Wformat=2
        -Wstrict-prototypes -Wmissing-prototypes
        -Wundef
        -Wvla
        -Wcast-align
        -Wnull-dereference
)
target_compile_options(2d_platformer PRIVATE
        "$<$<CONFIG:Debug>:-O0;-g>"
        "$<$<CONFIG:Release>:-O2;-Werror>"
)

# Platform layer
if(WIN32)
    target_sources(2d_platformer PRIVATE src/win32_main.c)
    target_link_libraries(2d_platformer PRIVATE user32)
    set_target_properties(2d_platformer PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(2d_platformer PRIVATE -static)
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED COMPONENTS Xext)
    target_sources(2d_platformer PRIVATE src/x11_main.c)
    target_link_libraries(2d_platformer PRIVATE X11::X11 X11::Xext m)
    target_link_options(2d_platformer PRIVATE -static-libgcc)
endif()