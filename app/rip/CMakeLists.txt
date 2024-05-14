project(rip)

# Build rip

set(rip_SOURCES RIPAssignment.cpp RIPAssignment.hpp)
add_library(rip SHARED ${rip_SOURCES})
target_link_libraries(rip PUBLIC e)


set(rip-targets rip)

if (TARGET rip-ref)
list(APPEND rip-targets rip-ref)
endif()


foreach(rip-traget ${rip-targets})
  add_executable(${rip-traget}-all testrip.cpp)
  target_link_libraries(${rip-traget}-all PUBLIC ${rip-traget} gtest_main)
  if(${CMAKE_VERSION} VERSION_GREATER "3.15.0")
    set_target_properties(${rip-traget}-all PROPERTIES XCODE_GENERATE_SCHEME ON)
    set_target_properties(${rip-traget}-all PROPERTIES XCODE_SCHEME_ARGUMENTS
                                                "--gtest_color=no")
    set_target_properties(${rip-traget}-all PROPERTIES XCODE_SCHEME_ENVIRONMENT
                                                "GTEST_COLOR=no")
  endif()
endforeach()
