project(routing)

# Build routing

set(routing_SOURCES RoutingAssignment.cpp RoutingAssignment.hpp testrouting.cpp)

add_executable(routing-all ${routing_SOURCES})
target_link_libraries(routing-all PUBLIC gtest_main)

if(NOT WIN32)
  target_compile_options(routing-all PUBLIC -g -O0 -Wall)
endif()

target_link_libraries(routing-all PUBLIC e)

if(${CMAKE_VERSION} VERSION_GREATER "3.15.0")
  set_target_properties(routing-all PROPERTIES XCODE_GENERATE_SCHEME ON)
  set_target_properties(routing-all PROPERTIES XCODE_SCHEME_ARGUMENTS
                                               "--gtest_color=no")
  set_target_properties(routing-all PROPERTIES XCODE_SCHEME_ENVIRONMENT
                                               "GTEST_COLOR=no")
endif()
