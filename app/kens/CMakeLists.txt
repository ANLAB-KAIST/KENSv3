project(kens)

# Build KENS

set(kens_SOURCES TCPAssignment.cpp TCPAssignment.hpp)

add_library(kens ${kens_SOURCES})

if(NOT WIN32)
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
set(kens_parts-all_SOURCES ${kens_part1_SOURCES} ${kens_part2_SOURCES}
                     ${kens_part3_SOURCES} ${kens_part4_SOURCES})

foreach(part part1 part2 part3 part4 parts-all)
  add_executable(${part} testenv.hpp ${kens_${part}_SOURCES})
  target_link_libraries(${part} kens)
  add_executable(${part}-unreliable testenv.hpp ${kens_${part}_SOURCES})
  target_link_libraries(${part}-unreliable kens)
  add_executable(${part}-run-solution testenv.hpp ${kens_${part}_SOURCES})
  target_link_libraries(${part}-run-solution kens)
  add_executable(${part}-unreliable-run-solution testenv.hpp ${kens_${part}_SOURCES})
  target_link_libraries(${part}-unreliable-run-solution kens)

  target_compile_definitions(${part}-run-solution PRIVATE RUN_SOLUTION)
  target_compile_definitions(${part}-unreliable-run-solution
                             PRIVATE RUN_SOLUTION UNRELIABLE)
  target_compile_definitions(${part}-unreliable PRIVATE UNRELIABLE)

  if(${CMAKE_VERSION} VERSION_GREATER "3.13.0")
    set_target_properties(
      ${part} ${part}-run-solution ${part}-unreliable
      ${part}-unreliable-run-solution PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(
      ${part} ${part}-run-solution ${part}-unreliable
      ${part}-unreliable-run-solution PROPERTIES XCODE_SCHEME_ENVIRONMENT
                                                      "GTEST_COLOR=no")
  endif()
endforeach(part)
