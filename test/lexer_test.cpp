#include "../src/lexer.hpp"
#include "../src/token.hpp"
#include "../third_party/catch.hpp"

TEST_CASE("Lexer generates tokens", "[lexer]") {
  std::string input = "hoge\n"
                      "hoo";
  yslang::Lexer lexer(input);

  yslang::Token token = lexer.next();
  REQUIRE(token.type == yslang::TokenType::Ident);
  REQUIRE(token.str == "hoge");

  token = lexer.next();
  REQUIRE(token.type == yslang::TokenType::Ident);
  REQUIRE(token.str == "hoo");

  token = lexer.next();
  REQUIRE(token.type == yslang::TokenType::TEOF);
  REQUIRE(token.str == "");
}

struct Case {
  yslang::TokenType type;
  std::string str;
};

TEST_CASE("Check all tokens", "[lexer]") {
  std::string input = "1 + hoge * 200 / foo();\n"
                      "hoo_bar";

  Case expected_tokens[] = {
    { yslang::TokenType::Integer, "1" },
    { yslang::TokenType::Plus, "" },
    { yslang::TokenType::Ident, "hoge" },
    { yslang::TokenType::Mul, "" },
    { yslang::TokenType::Integer, "200" },
    { yslang::TokenType::Div, "" },
    { yslang::TokenType::Ident, "foo" },
    { yslang::TokenType::ParenL, "" },
    { yslang::TokenType::ParenR, "" },
    { yslang::TokenType::Semicolon, "" },
    { yslang::TokenType::Ident, "hoo_bar" },
    { yslang::TokenType::TEOF, "" },
  };

  yslang::Lexer lexer(input);

  for (const auto &expected : expected_tokens) {
    yslang::Token token = lexer.next();
    REQUIRE(token.type == expected.type);
    REQUIRE(token.str == expected.str);
  }
}
