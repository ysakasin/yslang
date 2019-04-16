#include "../src/lexer.hpp"
#include "../src/token.hpp"
#include "../third_party/catch.hpp"

#define TEST_TOKEN(l, t, s)                                                    \
  {                                                                            \
    yslang::Token token = l.next();                                            \
    REQUIRE(token.type == t);                                                  \
    REQUIRE(token.str == s);                                                   \
  }

TEST_CASE("Lexer generates tokens", "[lexer]") {
  std::string input = "hoge\n"
                      "hoo";
  yslang::Lexer lexer(input);

  TEST_TOKEN(lexer, yslang::TokenType::Ident, "hoge");
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "hoo");
  TEST_TOKEN(lexer, yslang::TokenType::TEOF, "");
}

struct Case {
  yslang::TokenType type;
  std::string str;
};

TEST_CASE("Check all tokens", "[lexer]") {
  std::string input = "1 + hoge * 200 / foo();\n"
                      "hoo_bar";

  yslang::Lexer lexer(input);
  TEST_TOKEN(lexer, yslang::TokenType::Integer, "1");
  TEST_TOKEN(lexer, yslang::TokenType::Plus, "");
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "hoge");
  TEST_TOKEN(lexer, yslang::TokenType::Mul, "");
  TEST_TOKEN(lexer, yslang::TokenType::Integer, "200");
  TEST_TOKEN(lexer, yslang::TokenType::Div, "");
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "foo");
  TEST_TOKEN(lexer, yslang::TokenType::ParenL, "");
  TEST_TOKEN(lexer, yslang::TokenType::ParenR, "");
  TEST_TOKEN(lexer, yslang::TokenType::Semicolon, "");
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "hoo_bar");
  TEST_TOKEN(lexer, yslang::TokenType::TEOF, "");
}

TEST_CASE("Check all tokens2", "[lexer]") {
  std::string input = "func hoge(a int) int {\n"
                      "  return 1\n"
                      "}\n";

  yslang::Lexer lexer(input);

  TEST_TOKEN(lexer, yslang::TokenType::Func, "")
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "hoge")
  TEST_TOKEN(lexer, yslang::TokenType::ParenL, "")
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "a")
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "int")
  TEST_TOKEN(lexer, yslang::TokenType::ParenR, "")
  TEST_TOKEN(lexer, yslang::TokenType::Ident, "int")
  TEST_TOKEN(lexer, yslang::TokenType::BraceL, "")
  TEST_TOKEN(lexer, yslang::TokenType::Return, "")
  TEST_TOKEN(lexer, yslang::TokenType::Integer, "1")
  TEST_TOKEN(lexer, yslang::TokenType::BraceR, "")
  TEST_TOKEN(lexer, yslang::TokenType::TEOF, "")
}
