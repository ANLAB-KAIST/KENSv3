project(pwospf)

# Build pwospf

set(pwospf_SOURCES PWOSPFAssignment.cpp PWOSPFAssignment.hpp)
add_library(pwospf SHARED ${pwospf_SOURCES})
target_link_libraries(pwospf PUBLIC e)

set(pwospf-targets pwospf)

if(TARGET pwospf-ref)
  list(APPEND pwospf-targets pwospf-ref)
endif()

foreach(pwospf-traget ${pwospf-targets})
  add_executable(${pwospf-traget}-all testpwospf.cpp)
  target_link_libraries(${pwospf-traget}-all PUBLIC ${pwospf-traget} gtest_main)
  if(${CMAKE_VERSION} VERSION_GREATER "3.15.0")
    set_target_properties(${pwospf-traget}-all PROPERTIES XCODE_GENERATE_SCHEME
                                                          ON)
    set_target_properties(${pwospf-traget}-all PROPERTIES XCODE_SCHEME_ARGUMENTS
                                                          "--gtest_color=no")
    set_target_properties(${pwospf-traget}-all
                          PROPERTIES XCODE_SCHEME_ENVIRONMENT "GTEST_COLOR=no")
  endif()
endforeach()
