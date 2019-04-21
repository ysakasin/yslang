#include "../src/parser.hpp"
#include "../third_party/catch.hpp"

TEST_CASE("Parser generates AST", "[parser]") {
  std::string input = R"(
func hoge(a int) int {
  return 1 + 2;
})";

  std::string expected = R"({
  "kind": "Program",
  "path": "",
  "decls": [
    {
      "kind": "FuncDecl",
      "name": "hoge",
      "type": {
        "result": {
          "kind": "Ident",
          "name": "int"
        },
        "args": [
          {
            "name": "a",
            "type": "int"
          }
        ]
      },
      "body": {
        "kind": "BlockStmt",
        "stmts": [
          {
            "kind": "ReturnStmt",
            "results": [
              {
                "kind": "BinaryExpr",
                "lhs": {
                  "kind": "Integer",
                  "value": "1"
                },
                "op": "Plus",
                "rhs": {
                  "kind": "Integer",
                  "value": "2"
                }
              }
            ]
          }
        ]
      }
    }
  ]
})";

  yslang::Parser parser(input);
  yslang::Program program = parser.parse();

  if (parser.has_error()) {
    for (const auto &msg : parser.error_messages) {
      FAIL_CHECK(msg);
    }
    FAIL("Parser has errors");
  }

  REQUIRE(program.toJson().to_string() == expected);
}

TEST_CASE("Fibonacci", "[parser]") {
  std::string input = R"(
func fib(n i64) i64 {
  if n <= 1 {
    return n;
  } else {
    return fib(n - 1) + fib(n - 2);
  }
}

func main() i64 {
  return fib(11);
})";

  std::string expected = R"({
  "kind": "Program",
  "path": "",
  "decls": [
    {
      "kind": "FuncDecl",
      "name": "fib",
      "type": {
        "result": {
          "kind": "Ident",
          "name": "i64"
        },
        "args": [
          {
            "name": "n",
            "type": "i64"
          }
        ]
      },
      "body": {
        "kind": "BlockStmt",
        "stmts": [
          {
            "kind": "IfStmt",
            "cond": {
              "kind": "BinaryExpr",
              "lhs": {
                "kind": "Ident",
                "name": "n"
              },
              "op": "LessEqual",
              "rhs": {
                "kind": "Integer",
                "value": "1"
              }
            },
            "then_block": {
              "kind": "BlockStmt",
              "stmts": [
                {
                  "kind": "ReturnStmt",
                  "results": [
                    {
                      "kind": "Ident",
                      "name": "n"
                    }
                  ]
                }
              ]
            },
            "else_block": {
              "kind": "BlockStmt",
              "stmts": [
                {
                  "kind": "ReturnStmt",
                  "results": [
                    {
                      "kind": "BinaryExpr",
                      "lhs": {
                        "kind": "CallExpr",
                        "func": {
                          "kind": "Ident",
                          "name": "fib"
                        },
                        "args": [
                          {
                            "kind": "BinaryExpr",
                            "lhs": {
                              "kind": "Ident",
                              "name": "n"
                            },
                            "op": "Minus",
                            "rhs": {
                              "kind": "Integer",
                              "value": "1"
                            }
                          }
                        ]
                      },
                      "op": "Plus",
                      "rhs": {
                        "kind": "CallExpr",
                        "func": {
                          "kind": "Ident",
                          "name": "fib"
                        },
                        "args": [
                          {
                            "kind": "BinaryExpr",
                            "lhs": {
                              "kind": "Ident",
                              "name": "n"
                            },
                            "op": "Minus",
                            "rhs": {
                              "kind": "Integer",
                              "value": "2"
                            }
                          }
                        ]
                      }
                    }
                  ]
                }
              ]
            }
          }
        ]
      }
    },
    {
      "kind": "FuncDecl",
      "name": "main",
      "type": {
        "result": {
          "kind": "Ident",
          "name": "i64"
        },
        "args": []
      },
      "body": {
        "kind": "BlockStmt",
        "stmts": [
          {
            "kind": "ReturnStmt",
            "results": [
              {
                "kind": "CallExpr",
                "func": {
                  "kind": "Ident",
                  "name": "fib"
                },
                "args": [
                  {
                    "kind": "Integer",
                    "value": "11"
                  }
                ]
              }
            ]
          }
        ]
      }
    }
  ]
})";

  yslang::Parser parser(input);
  yslang::Program program = parser.parse();

  if (parser.has_error()) {
    for (const auto &msg : parser.error_messages) {
      FAIL_CHECK(msg);
    }
    FAIL("Parser has errors");
  }

  REQUIRE(program.toJson().to_string() == expected);
}

TEST_CASE("Let statement", "[parser]") {
  std::string input = R"(
func hoge(a int) int {
  let x = 10;
  return x;
})";

  std::string expected = R"({
  "kind": "Program",
  "path": "",
  "decls": [
    {
      "kind": "FuncDecl",
      "name": "hoge",
      "type": {
        "result": {
          "kind": "Ident",
          "name": "int"
        },
        "args": [
          {
            "name": "a",
            "type": "int"
          }
        ]
      },
      "body": {
        "kind": "BlockStmt",
        "stmts": [
          {
            "kind": "LetStmt",
            "ident": {
              "kind": "Ident",
              "name": "x"
            },
            "expr": {
              "kind": "Integer",
              "value": "10"
            }
          },
          {
            "kind": "ReturnStmt",
            "results": [
              {
                "kind": "Ident",
                "name": "x"
              }
            ]
          }
        ]
      }
    }
  ]
})";

  yslang::Parser parser(input);
  yslang::Program program = parser.parse();

  if (parser.has_error()) {
    for (const auto &msg : parser.error_messages) {
      FAIL_CHECK(msg);
    }
    FAIL("Parser has errors");
  }

  REQUIRE(program.toJson().to_string() == expected);
}
