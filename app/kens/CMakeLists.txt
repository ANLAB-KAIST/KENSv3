project(kens)

# Build KENS

set(kens_SOURCES TCPAssignment.cpp TCPAssignment.hpp)

add_library(kens SHARED ${kens_SOURCES})

target_link_libraries(kens PUBLIC e)

get_solution(kens)

# Tests

set(kens_part1_SOURCES testopen.cpp testbind.cpp)
set(kens_part2_SOURCES testhandshake.cpp testclose.cpp)
set(kens_part3_SOURCES testtransfer.cpp)
set(kens_part4_SOURCES testcongestion.cpp)
set(kens_all_SOURCES ${kens_part1_SOURCES} ${kens_part2_SOURCES}
                     ${kens_part3_SOURCES} ${kens_part4_SOURCES})


set(kens-targets kens)

if (TARGET kens-ref)
list(APPEND kens-targets kens-ref)
endif()


foreach(kens-traget ${kens-targets})
  foreach(part part1 part2 part3 part4 all)
    add_executable(${kens-traget}-${part} testenv.hpp ${kens_${part}_SOURCES})
    target_link_libraries(${kens-traget}-${part} ${kens-traget} kens_solution gtest_main)
    add_executable(${kens-traget}-${part}-unreliable testenv.hpp ${kens_${part}_SOURCES})
    target_link_libraries(${kens-traget}-${part}-unreliable ${kens-traget} kens_solution gtest_main)
    add_executable(${kens-traget}-${part}-run-solution testenv.hpp ${kens_${part}_SOURCES})
    target_link_libraries(${kens-traget}-${part}-run-solution ${kens-traget} kens_solution gtest_main)
    add_executable(${kens-traget}-${part}-unreliable-run-solution testenv.hpp
                                                        ${kens_${part}_SOURCES})
    target_link_libraries(${kens-traget}-${part}-unreliable-run-solution ${kens-traget} kens_solution gtest_main)

    target_compile_definitions(${kens-traget}-${part}-run-solution PRIVATE RUN_SOLUTION)
    target_compile_definitions(${kens-traget}-${part}-unreliable-run-solution
                              PRIVATE RUN_SOLUTION UNRELIABLE)
    target_compile_definitions(${kens-traget}-${part}-unreliable PRIVATE UNRELIABLE)

    if(${CMAKE_VERSION} VERSION_GREATER "3.13.0")
      set_target_properties(
        ${kens-traget}-${part} ${kens-traget}-${part}-run-solution ${kens-traget}-${part}-unreliable
        ${kens-traget}-${part}-unreliable-run-solution PROPERTIES XCODE_GENERATE_SCHEME ON)
      set_target_properties(
        ${kens-traget}-${part} ${kens-traget}-${part}-run-solution ${kens-traget}-${part}-unreliable
        ${kens-traget}-${part}-unreliable-run-solution PROPERTIES XCODE_SCHEME_ENVIRONMENT
                                                        "GTEST_COLOR=no")
    endif()
  endforeach(part)
endforeach(kens-traget)
