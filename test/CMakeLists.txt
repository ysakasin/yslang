set(test_src
  test.cpp
  lexer_test.cpp
  parser_test.cpp
)

add_executable(tester ${test_src})
add_custom_target(
  test
  ${CMAKE_BINARY_DIR}/tester
)

target_link_libraries(tester yslang)
