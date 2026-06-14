cmake_minimum_required(VERSION 3.14)
project(rogueterm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address,undefined")

add_executable(rogueterm
  src/main.cpp
  src/rng.cpp
  src/map.cpp
  src/fov.cpp
  src/entity.cpp
  src/items.cpp
  src/combat.cpp
  src/input.cpp
  src/renderer.cpp
  src/game.cpp
)

target_include_directories(rogueterm PRIVATE src)
target_compile_options(rogueterm PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS rogueterm RUNTIME DESTINATION bin)
