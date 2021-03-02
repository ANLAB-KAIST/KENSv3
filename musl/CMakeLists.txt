project(musl_compat VERSION 3.1)

file(GLOB_RECURSE musl_compat_SOURCES "src/*.c")
file(GLOB_RECURSE musl_compat_HEADERS "include/*.h")

add_library(${PROJECT_NAME} STATIC ${musl_compat_SOURCES} ${musl_compat_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-shift-op-parentheses)