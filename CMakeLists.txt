cmake_minimum_required(VERSION 3.11)

project(e VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
# Build E
file(GLOB_RECURSE e_SOURCES "src/*.cpp")
file(GLOB_RECURSE e_HEADERS "include/*.hpp")

add_library(e STATIC ${e_SOURCES} ${e_HEADERS})
target_include_directories(e PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(e PRIVATE -g -O0 -Wall)

# Find Solution

function(get_solution_name TARGET)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_SYSTEM_NAME STREQUAL
                                                     "Darwin")
    set(${TARGET}_SOLUTION_NAME
        "${TARGET}-universal-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}.zip"
        PARENT_SCOPE)
  else()
    set(${TARGET}_SOLUTION_NAME
        "${TARGET}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}.zip"
        PARENT_SCOPE)
  endif()
endfunction()

# Build Solutions

file(GLOB_RECURSE e_solution_CMAKELISTS
     "${PROJECT_SOURCE_DIR}/solution/*/CMakeLists.txt")
foreach(cmakelists ${e_solution_CMAKELISTS})
  get_filename_component(e_solution_DIR ${cmakelists} DIRECTORY)
  add_subdirectory(${e_solution_DIR})
endforeach()

# Solution Downloader

include(FetchContent)
function(get_solution)
  set(SOLUTION_REPO "https://anlab-kaist.github.io/KENSv3/solution")
  foreach(arg IN LISTS ARGN)
    if(NOT TARGET ${arg}_solution)

      get_solution_name(${arg}_solution)

      include(FetchContent)

      FetchContent_Declare(
        ${arg}_solution URL "${SOLUTION_REPO}/${${arg}_solution_SOLUTION_NAME}")
      FetchContent_GetProperties(${arg}_solution)
      if(NOT (${arg}_solution_POPULATED))
        FetchContent_Populate(${arg}_solution)
      endif()

      file(GLOB ${arg}_solution_static "${${arg}_solution_SOURCE_DIR}/*.a")

      add_library(${arg}_solution STATIC IMPORTED GLOBAL)
      set_property(TARGET ${arg}_solution PROPERTY IMPORTED_LOCATION
                                                   ${${arg}_solution_static})

      target_link_libraries(${arg}_solution INTERFACE e)

    endif()

  endforeach()
endfunction()

# Build Apps

file(GLOB e_apps_CMAKELISTS "${PROJECT_SOURCE_DIR}/app/*/CMakeLists.txt")
foreach(cmakelists ${e_apps_CMAKELISTS})
  get_filename_component(e_apps_DIR ${cmakelists} DIRECTORY)
  add_subdirectory(${e_apps_DIR})
endforeach()

# Doxygen

find_package(Doxygen)
if(DOXYGEN_FOUND)

  add_custom_target(
    doxygen
    COMMAND
      ${CMAKE_COMMAND} -E env
      "DOXYGEN_OUTPUT_DIRECTORY=${PROJECT_BINARY_DIR}/doc" doxygen
      ${PROJECT_SOURCE_DIR}/doxygen/Doxyfile
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

endif(DOXYGEN_FOUND)

# KENS dist

set(kens_dist_path ${PROJECT_BINARY_DIR}/dist/kens)

add_custom_target(
  dist_kens
  COMMAND ${CMAKE_COMMAND} -E make_directory ${kens_dist_path}
  COMMAND
    ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/.clang-format
    ${PROJECT_SOURCE_DIR}/.gitignore ${PROJECT_SOURCE_DIR}/CMakeLists.txt
    ${PROJECT_SOURCE_DIR}/Dockerfile ${PROJECT_SOURCE_DIR}/entrypoint.sh
    ${PROJECT_SOURCE_DIR}/LICENSE ${kens_dist_path}
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/README_KENS.md
          ${kens_dist_path}/README.md
  COMMAND ${CMAKE_COMMAND} -E make_directory ${kens_dist_path}/app/kens
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/app/kens
          ${kens_dist_path}/app/kens
  COMMAND ${CMAKE_COMMAND} -E make_directory ${kens_dist_path}/doxygen
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/doxygen
          ${kens_dist_path}/doxygen
  COMMAND ${CMAKE_COMMAND} -E make_directory ${kens_dist_path}/include
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/include
          ${kens_dist_path}/include
  COMMAND ${CMAKE_COMMAND} -E make_directory ${kens_dist_path}/src
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/src
          ${kens_dist_path}/src)
