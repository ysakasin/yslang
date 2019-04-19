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
                "kind": "Ident",
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
