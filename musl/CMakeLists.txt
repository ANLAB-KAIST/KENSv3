project(musl_compat VERSION 3.1)

set(musl_version 1.2.2)

include(FetchContent)

FetchContent_Declare(
    musl URL "https://musl.libc.org/releases/musl-${musl_version}.tar.gz")

FetchContent_GetProperties(musl)
if(NOT (musl_POPULATED))
    FetchContent_Populate(musl)
endif()
set(musl_HEADERS 
    "netinet/*.h"
    "net/*.h"
    "arpa/*.h"
    "byteswap.h"
    "features.h"
    "sys/socket.h"
)
set(musl_SRC 
    "${musl_SOURCE_DIR}/src/network/ether.c"
    "${musl_SOURCE_DIR}/src/network/htonl.c"
    "${musl_SOURCE_DIR}/src/network/htons.c"
    "${musl_SOURCE_DIR}/src/network/inet_addr.c"
    "${musl_SOURCE_DIR}/src/network/inet_aton.c"
    "${musl_SOURCE_DIR}/src/network/inet_legacy.c"
    "${musl_SOURCE_DIR}/src/network/inet_ntoa.c"
    "${musl_SOURCE_DIR}/src/network/inet_ntop.c"
    "${musl_SOURCE_DIR}/src/network/inet_pton.c"
    "${musl_SOURCE_DIR}/src/network/ntohl.c"
    "${musl_SOURCE_DIR}/src/network/ntohs.c"
)


foreach(header ${musl_HEADERS})
file(INSTALL ${musl_SOURCE_DIR}/include DESTINATION ${PROJECT_BINARY_DIR}
    FILES_MATCHING
    PATTERN ${header}
)
endforeach()


file(GLOB_RECURSE musl_compat_HEADERS "include/*.h" "${PROJECT_BINARY_DIR}/include/*.h")
add_library(${PROJECT_NAME} STATIC ${musl_SRC} ${musl_compat_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)


# Handle weak alias
target_compile_definitions(${PROJECT_NAME} PRIVATE __inet_aton=inet_aton)


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-shift-op-parentheses)
endif()

add_executable(musl_compat_test test.c)
target_link_libraries(musl_compat_test ${PROJECT_NAME})