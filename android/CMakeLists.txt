cmake_minimum_required(VERSION 3.22.1)
project(soniccore_android LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SONICCORE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(../core ${CMAKE_CURRENT_BINARY_DIR}/soniccore_core)

add_library(soniccore_jni SHARED
    jni/soniccore_jni.cpp
)

target_include_directories(soniccore_jni PRIVATE ../core/include)
target_compile_options(soniccore_jni PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(soniccore_jni PRIVATE soniccore log android)
