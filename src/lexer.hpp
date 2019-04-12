#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace yslang {
class Token;
enum class TokenType;

class Lexer {
public:
  Lexer(const std::string &input);
  Token next();

private:
  void skip_blank();
  void read_char();
  char peek_char();

  Token read_number();
  Token read_ident();
  Token read_string_lit();

private:
  static std::map<std::string, TokenType> keywords;
  static void init_keywords();

private:
  std::string input;
  std::string path;
  size_t position = 0;
  size_t read_position = 0;
  char ch;
};
} // namespace yslang
