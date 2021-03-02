project(kens)

# Get Library: GTest

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.10.0.zip)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR}/googletest ${googletest_BINARY_DIR})
endif()

# Build KENS

set(kens_SOURCES TCPAssignment.cpp TCPAssignment.hpp testenv.hpp)

add_library(kens ${kens_SOURCES})

if (NOT WIN32)
target_compile_options(kens PUBLIC -g -O0 -Wall)
endif()

target_link_libraries(kens PUBLIC e)

get_solution(kens)

target_link_libraries(kens PUBLIC kens_solution)
target_link_libraries(kens PUBLIC gtest_main)

# Tests

set(kens_part1_SOURCES testopen.cpp testbind.cpp)
set(kens_part2_SOURCES testhandshake.cpp testclose.cpp)
set(kens_part3_SOURCES testtransfer.cpp)
set(kens_part4_SOURCES testcongestion.cpp)

add_executable(kens-part1 ${kens_part1_SOURCES})
target_link_libraries(kens-part1 kens)

add_executable(kens-part2 ${kens_part2_SOURCES})
target_link_libraries(kens-part2 kens)

add_executable(kens-part3 ${kens_part3_SOURCES})
target_link_libraries(kens-part3 kens)

add_executable(kens-part4 ${kens_part4_SOURCES})
target_link_libraries(kens-part4 kens)

add_executable(kens-all ${kens_part1_SOURCES} ${kens_part2_SOURCES}
                        ${kens_part3_SOURCES} ${kens_part4_SOURCES})
target_link_libraries(kens-all kens)

add_executable(kens-part1-run-solution ${kens_part1_SOURCES})
target_link_libraries(kens-part1-run-solution kens)

add_executable(kens-part2-run-solution ${kens_part2_SOURCES})
target_link_libraries(kens-part2-run-solution kens)

add_executable(kens-part3-run-solution ${kens_part3_SOURCES})
target_link_libraries(kens-part3-run-solution kens)

add_executable(kens-part4-run-solution ${kens_part4_SOURCES})
target_link_libraries(kens-part4-run-solution kens)

add_executable(
  kens-all-run-solution ${kens_part1_SOURCES} ${kens_part2_SOURCES}
                        ${kens_part3_SOURCES} ${kens_part4_SOURCES})
target_link_libraries(kens-all-run-solution kens)

target_compile_definitions(kens-part1-run-solution PRIVATE RUN_SOLUTION)
target_compile_definitions(kens-part2-run-solution PRIVATE RUN_SOLUTION)
target_compile_definitions(kens-part3-run-solution PRIVATE RUN_SOLUTION)
target_compile_definitions(kens-part4-run-solution PRIVATE RUN_SOLUTION)
target_compile_definitions(kens-all-run-solution PRIVATE RUN_SOLUTION)

# XCode does not support color

if(${CMAKE_VERSION} VERSION_GREATER "3.15.0")
  set_target_properties(
    kens-part1
    kens-part2
    kens-part3
    kens-part4
    kens-all
    kens-part1-run-solution
    kens-part2-run-solution
    kens-part3-run-solution
    kens-part4-run-solution
    kens-all-run-solution
    PROPERTIES XCODE_GENERATE_SCHEME ON)
  set_target_properties(
    kens-part1
    kens-part2
    kens-part3
    kens-part4
    kens-all
    kens-part1-run-solution
    kens-part2-run-solution
    kens-part3-run-solution
    kens-part4-run-solution
    kens-all-run-solution
    PROPERTIES XCODE_SCHEME_ARGUMENTS "--gtest_color=no")

endif()
