#include "./token.hpp"

using namespace yslang;

std::ostream &operator<<(std::ostream &out, const TokenType type) {
  switch (type) {
  case TokenType::Integer:
    return out << "Integer";
  case TokenType::String:
    return out << "String";
  case TokenType::Ident:
    return out << "Ident";

  // keywords
  case TokenType::Const:
    return out << "Const";
  case TokenType::Let:
    return out << "Let";
  case TokenType::Func:
    return out << "Func";
  case TokenType::If:
    return out << "If";
  case TokenType::While:
    return out << "While";
  case TokenType::Return:
    return out << "Return";

  case TokenType::Plus:
    return out << "Plus";
  case TokenType::Minus:
    return out << "Minus";
  case TokenType::Mul:
    return out << "Mul";
  case TokenType::Div:
    return out << "Div";

  case TokenType::Equal:
    return out << "Equal";
  case TokenType::NotEqual:
    return out << "NotEqual";
  case TokenType::Less:
    return out << "Less";
  case TokenType::LessEqual:
    return out << "LessEqual";
  case TokenType::Greater:
    return out << "Greater";
  case TokenType::GreaterEqual:
    return out << "GreaterEqual";

  case TokenType::Assign:
    return out << "Assign";
  case TokenType::Semicolon:
    return out << "Semicolon";
  case TokenType::Colon:
    return out << "Colon";
  case TokenType::Comma:
    return out << "Comma";
  case TokenType::ParenL:
    return out << "ParenL";
  case TokenType::ParenR:
    return out << "ParenR";
  case TokenType::BraceL:
    return out << "BraceL";
  case TokenType::BraceR:
    return out << "BraceR";
  case TokenType::Dot:
    return out << "Dot";
  case TokenType::NewLine:
    return out << "NewLine";

  case TokenType::TEOF:
    return out << "EOF";
  }
}

std::ostream &operator<<(std::ostream &out, const Token &token) {
  out << token.type;
  if (token.type == TokenType::Integer || token.type == TokenType::String ||
      token.type == TokenType::Ident) {
    out << " " << token.str;
  }
  return out;
}
