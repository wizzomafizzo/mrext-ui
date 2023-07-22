cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER "/opt/gcc-arm-10.2-2020.11-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

project(radical C)

add_compile_options(-Wall -Wextra -O2 -g) # -Werror
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(SOURCES lib/fbg/nanojpeg/nanojpeg.c lib/fbg/fbgraphics.c lib/fbg/fbdev/fbg_fbdev.c
        lib/fbg/lodepng/lodepng.c lib/cwalk/cwalk.c
        src/screenshots.c src/console.c src/assets/assets.c src/scanners.c src/input.c)
add_executable(screenshots ${SOURCES})
include_directories(screenshots PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(screenshots m)
