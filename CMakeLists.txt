cmake_minimum_required(VERSION 3.0)
project(Arta)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_custom_target(triangle COMMAND make WORKING_DIRECTORY
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/triangle")

find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIRS})
link_libraries(${LUA_LIBRARIES})
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
link_libraries(${PNG_LIBRARIES})

file(GLOB_RECURSE lib_src "${CMAKE_CURRENT_SOURCE_DIR}" "src/*.cpp")
list(REMOVE_ITEM lib_src "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(arta ${lib_src})

add_executable(arta.exe "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(arta.exe arta)
add_dependencies(arta.exe triangle)
