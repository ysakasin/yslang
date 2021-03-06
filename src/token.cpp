#include "./token.hpp"
#include <sstream>

using namespace yslang;

bool Token::isOP() const {
  return type == TokenType::Plus || type == TokenType::Minus ||
         type == TokenType::Mul || type == TokenType::Div ||
         type == TokenType::Equal || type == TokenType::NotEqual ||
         type == TokenType::Greater || type == TokenType::GreaterEqual ||
         type == TokenType::Less || type == TokenType::LessEqual;
}

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
  case TokenType::Else:
    return out << "Else";
  case TokenType::While:
    return out << "While";
  case TokenType::Return:
    return out << "Return";
  case TokenType::Import:
    return out << "Import";
  case TokenType::Struct:
    return out << "Struct";
  case TokenType::Type:
    return out << "Type";

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
  case TokenType::LetAssign:
    return out << "LetAssign";
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
  case TokenType::BracketL:
    return out << "BracketL";
  case TokenType::BracketR:
    return out << "BracketR";
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
