cmake_minimum_required(VERSION 3.16)
project(dumok_football_manager_v02 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5)
FetchContent_MakeAvailable(raylib)
add_executable(dumok_fm main.cpp)
target_link_libraries(dumok_fm PRIVATE raylib)
if(MSVC)
  target_compile_options(dumok_fm PRIVATE /utf-8)
endif()

if(EMSCRIPTEN)
  set_target_properties(dumok_fm PROPERTIES SUFFIX ".html")
  target_link_options(dumok_fm PRIVATE
    "-sUSE_GLFW=3"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sASSERTIONS=1"
    "-sWASM=1"
  )
endif()
