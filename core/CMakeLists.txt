cmake_minimum_required(VERSION 3.22)
project(soniccore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(soniccore STATIC
    src/sonic_core.cpp
)

target_include_directories(soniccore PUBLIC include)
target_compile_options(soniccore PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(soniccore PROPERTIES POSITION_INDEPENDENT_CODE ON)

include(CTest)
option(SONICCORE_BUILD_TESTS "Build SonicCore offline tests" ON)
if(BUILD_TESTING AND SONICCORE_BUILD_TESTS)
    add_executable(soniccore_smoke_test ../tests/smoke_test.cpp)
    target_link_libraries(soniccore_smoke_test PRIVATE soniccore)
    add_test(NAME soniccore_smoke_test COMMAND soniccore_smoke_test)

    add_executable(soniccore_quality_test ../tests/quality_test.cpp)
    target_link_libraries(soniccore_quality_test PRIVATE soniccore)
    add_test(NAME soniccore_quality_test COMMAND soniccore_quality_test)

    add_executable(soniccore_golden_test ../tests/golden_test.cpp)
    target_link_libraries(soniccore_golden_test PRIVATE soniccore)
    add_test(NAME soniccore_golden_test COMMAND soniccore_golden_test)
endif()
