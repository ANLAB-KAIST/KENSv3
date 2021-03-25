cmake_minimum_required(VERSION 3.11)

project(e VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

# Compiler Test

include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
  "
    class a{
    int f(int leverl, const char *format, ...)
    __attribute__((format(printf, 3, 4)));
    };
    int main(void) {return 0;}
    "
  HAVE_ATTR_FORMAT)

if(HAVE_ATTR_FORMAT)
  add_compile_definitions(HAVE_ATTR_FORMAT)
endif()

check_cxx_source_compiles(
  "
    struct a{
    char b;
    long c;
    } __attribute__((packed));
    int main(void) {
      struct a a1;
      return 0;
    }
    "
  HAVE_ATTR_PACK)

check_cxx_source_compiles(
  "
    #pragma pack(push, 1)
    struct a{
      char b;
      long c;
    };
    #pragma pack(pop)
    int main(void) {
      struct a a1;
      return 0;
    }
    "
  HAVE_PRAGMA_PACK)

if(HAVE_ATTR_PACK)
  add_compile_definitions(HAVE_ATTR_PACK)
elseif(HAVE_PRAGMA_PACK)
  add_compile_definitions(HAVE_PRAGMA_PACK)
else()
  message(FATAL_ERROR "Compiler must support packing")
endif()

# Build E
file(GLOB_RECURSE e_SOURCES "src/*.cpp")
file(GLOB_RECURSE e_HEADERS "include/*.hpp")

add_library(e STATIC ${e_SOURCES} ${e_HEADERS})
target_include_directories(e PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(NOT WIN32)
  target_compile_options(e PRIVATE -g -O0 -Wall)
endif()

if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  add_subdirectory(musl)
  target_link_libraries(e PUBLIC musl_compat)

  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
endif(WIN32)

# Find Solution
function(get_solution_name TARGET)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_SYSTEM_NAME STREQUAL
                                                     "Darwin")
    set(${TARGET}_SOLUTION_NAME
        "${TARGET}-universal-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}.zip"
        PARENT_SCOPE)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_SYSTEM_NAME STREQUAL
                                                   "Windows")
    set(${TARGET}_SOLUTION_NAME
        "${TARGET}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-Clang.zip"
        PARENT_SCOPE)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(REPLACE "." ";" CMAKE_CXX_COMPILER_VERSION_LIST
                   "${CMAKE_CXX_COMPILER_VERSION}")
    list(GET CMAKE_CXX_COMPILER_VERSION_LIST 0 CMAKE_CXX_COMPILER_VERSION_MAJOR)
    set(${TARGET}_SOLUTION_NAME
        "${TARGET}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION_MAJOR}.zip"
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

      if(WIN32)
        file(GLOB ${arg}_solution_static "${${arg}_solution_SOURCE_DIR}/*.lib")
      else()
        file(GLOB ${arg}_solution_static "${${arg}_solution_SOURCE_DIR}/*.a")
      endif()

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

include(cmake/kens-dist.cmake OPTIONAL)
