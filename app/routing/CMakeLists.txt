project(routing)

# Build routing

set(routing_SOURCES RoutingAssignment.cpp RoutingAssignment.hpp)
add_library(routing SHARED ${routing_SOURCES})
target_link_libraries(routing PUBLIC e)


set(routing-targets routing)

if (TARGET routing-ref)
list(APPEND routing-targets routing-ref)
endif()


foreach(routing-traget ${routing-targets})
  add_executable(${routing-traget}-all testrouting.cpp)
  target_link_libraries(${routing-traget}-all PUBLIC ${routing-traget} gtest_main)
  if(${CMAKE_VERSION} VERSION_GREATER "3.15.0")
    set_target_properties(${routing-traget}-all PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(${routing-traget}-all PROPERTIES XCODE_SCHEME_ARGUMENTS
                                                "--gtest_color=no")
    set_target_properties(${routing-traget}-all PROPERTIES XCODE_SCHEME_ENVIRONMENT
                                                "GTEST_COLOR=no")
  endif()
endforeach()
