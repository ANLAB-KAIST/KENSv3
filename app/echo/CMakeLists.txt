project(echo)

set(echo_SOURCES EchoAssignment.cpp EchoAssignment.hpp)

add_executable(echo ${echo_SOURCES} testecho.cpp)
add_executable(echo-non-kens ${echo_SOURCES} EchoNonKens.cpp)

get_solution(kens)

target_link_libraries(echo kens kens_solution gtest_main)
target_compile_definitions(echo-non-kens PRIVATE NON_KENS)
