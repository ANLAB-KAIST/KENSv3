project(kens)

# Build KENS

set(kens_SOURCES TCPAssignment.cpp TCPAssignment.hpp)

add_library(kens SHARED ${kens_SOURCES})

target_link_libraries(kens PUBLIC e)

get_solution(kens)

# Tests

set(test_open_SOURCES testopen.cpp)
set(test_bind_SOURCES testbind.cpp)
set(test_handshake_SOURCES testhandshake.cpp)
set(test_close_SOURCES testclose.cpp)
set(test_transfer_SOURCES testtransfer.cpp)
set(test_congestion_SOURCES testcongestion.cpp)
set(test_all_SOURCES testopen.cpp testbind.cpp testhandshake.cpp testclose.cpp
                     testtransfer.cpp testcongestion.cpp)

set(kens-targets kens)

if(TARGET kens-ref)
  list(APPEND kens-targets kens-ref)
endif()

foreach(kens-traget ${kens-targets})
  foreach(
    part
    open
    bind
    handshake
    close
    transfer
    congestion
    all)

    add_executable(test-${kens-traget}-${part} testenv.hpp
                                               ${test_${part}_SOURCES})
    target_link_libraries(test-${kens-traget}-${part} ${kens-traget}
                          kens_solution gtest_main)
    add_executable(test-${kens-traget}-${part}-unreliable
                   testenv.hpp ${test_${part}_SOURCES})
    target_link_libraries(test-${kens-traget}-${part}-unreliable ${kens-traget}
                          kens_solution gtest_main)
    add_executable(test-${kens-traget}-${part}-run-solution
                   testenv.hpp ${test_${part}_SOURCES})
    target_link_libraries(test-${kens-traget}-${part}-run-solution
                          ${kens-traget} kens_solution gtest_main)
    add_executable(test-${kens-traget}-${part}-unreliable-run-solution
                   testenv.hpp ${test_${part}_SOURCES})
    target_link_libraries(test-${kens-traget}-${part}-unreliable-run-solution
                          ${kens-traget} kens_solution gtest_main)

    target_compile_definitions(test-${kens-traget}-${part}-run-solution
                               PRIVATE RUN_SOLUTION)
    target_compile_definitions(
      test-${kens-traget}-${part}-unreliable-run-solution PRIVATE RUN_SOLUTION
                                                                  UNRELIABLE)
    target_compile_definitions(test-${kens-traget}-${part}-unreliable
                               PRIVATE UNRELIABLE)

    if(${CMAKE_VERSION} VERSION_GREATER "3.13.0")
      set_target_properties(
        test-${kens-traget}-${part} test-${kens-traget}-${part}-run-solution
        test-${kens-traget}-${part}-unreliable
        test-${kens-traget}-${part}-unreliable-run-solution
        PROPERTIES XCODE_GENERATE_SCHEME ON)
      set_target_properties(
        test-${kens-traget}-${part} test-${kens-traget}-${part}-run-solution
        test-${kens-traget}-${part}-unreliable
        test-${kens-traget}-${part}-unreliable-run-solution
        PROPERTIES XCODE_SCHEME_ENVIRONMENT "GTEST_COLOR=no")
    endif()
  endforeach(part)
endforeach(kens-traget)
