#pragma once

#include <ostream>
#include <string>

namespace yslang {
enum class TokenType {
  Integer,
  String,
  Ident,

  // keywords
  Const,
  Let,
  Func,
  If,
  Else,
  While,
  Return,
  Import,
  Struct,
  Type,

  Plus,  // +
  Minus, // -
  Mul,   // *
  Div,   // /

  Equal,        // ==
  NotEqual,     // !=
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=

  Assign,    // =
  LetAssign, // :=
  Semicolon, // ;
  Colon,     // :
  Comma,     // ,
  ParenL,    // (
  ParenR,    // )
  BraceL,    // {
  BraceR,    // }
  BracketL,  // [
  BracketR,  // ]
  Dot,       // .
  NewLine,   // \n

  TEOF,
};

class Token {
public:
  Token() : type(TokenType::TEOF) {}
  Token(TokenType type) : type(type) {}
  Token(TokenType type, std::string &&str) : type(type), str(str) {}

  bool isEOF() const {
    return type == TokenType::TEOF;
  }
  bool isOP() const;

public:
  TokenType type;
  std::string str;
};

} // namespace yslang

std::ostream &operator<<(std::ostream &out, const yslang::TokenType type);
std::ostream &operator<<(std::ostream &out, const yslang::Token &token);
