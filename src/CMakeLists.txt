set(yslang_src
  ast.cpp
  # codegen.cpp
  lexer.cpp
  parser.cpp
  token.cpp
)

add_library(yslang STATIC ${yslang_src})
