#include "./lexer.hpp"
#include "./token.hpp"

using namespace yslang;

std::map<std::string, TokenType> Lexer::keywords;

Lexer::Lexer(const std::string &input) : input(input) {
  Lexer::init_keywords();
  read_char();
}

Token Lexer::next() {
  Token token;

  skip_blank();

  switch (this->ch) {
  case '0' ... '9':
    return read_number();
  case 'a' ... 'z':
  case 'A' ... 'Z':
  case '_':
    return read_ident();
  case '"':
    return read_string_lit();
  case '\0':
    token.type = TokenType::TEOF;
    break;
  case '+':
    token.type = TokenType::Plus;
    break;
  case '-':
    token.type = TokenType::Minus;
    break;
  case '*':
    token.type = TokenType::Mul;
    break;
  case '/':
    token.type = TokenType::Div;
    break;
  case '=':
    if (peek_char() == '=') {
      read_char();
      token.type = TokenType::Equal;
    } else {
      token.type = TokenType::Assign;
    }
    break;
  case '<':
    if (peek_char() == '=') {
      read_char();
      token.type = TokenType::LessEqual;
    } else {
      token.type = TokenType::Less;
    }
    break;
  case '>':
    if (peek_char() == '=') {
      read_char();
      token.type = TokenType::GreaterEqual;
    } else {
      token.type = TokenType::Greater;
    }
    break;
  case '(':
    token.type = TokenType::ParenL;
    break;
  case ')':
    token.type = TokenType::ParenR;
    break;
  case '{':
    token.type = TokenType::BraceL;
    break;
  case '}':
    token.type = TokenType::BraceR;
    break;
  case '.':
    token.type = TokenType::Dot;
    break;
  case ':':
    if (peek_char() == '=') {
      read_char();
      token.type = TokenType::Assign;
    } else {
      token.type = TokenType::Colon;
    }
    break;
  case ';':
    token.type = TokenType::Semicolon;
    break;
  case ',':
    token.type = TokenType::Comma;
    break;
  default:
    throw "invalid char: " + std::string{ this->ch };
  }

  read_char();
  return token;
}

void Lexer::skip_blank() {
  // skip ' ', '\n', \t', '\v', '\f', '\r'
  while (isspace(this->ch)) {
    read_char();
  }
}

void Lexer::read_char() {
  if (read_position >= input.size()) {
    this->ch = '\0';
  } else {
    this->ch = input[read_position];
  }
  position = read_position++;
}

char Lexer::peek_char() {
  if (read_position >= input.size()) {
    return '\0';
  } else {
    return input[read_position];
  }
}

Token Lexer::read_number() {
  size_t pos = position;

  while (isdigit(this->ch)) {
    read_char();
  }
  return Token(TokenType::Integer, input.substr(pos, position - pos));
}

bool is_ident_piece(char c) {
  return isalnum(c) || c == '_';
}

Token Lexer::read_ident() {
  size_t pos = position;

  while (is_ident_piece(this->ch)) {
    read_char();
  }

  std::string ident = input.substr(pos, position - pos);

  auto itr = keywords.find(ident);
  if (itr == keywords.end()) {
    return Token(TokenType::Ident, std::move(ident));
  } else {
    return Token(itr->second);
  }
}

Token Lexer::read_string_lit() {
  size_t pos = position + 1;

  while (this->ch != '"') {
    read_char();
  }

  return Token(TokenType::String, input.substr(pos, position - pos - 1));
}

void Lexer::init_keywords() {
  if (keywords.size() != 0) {
    return;
  }
  keywords["const"] = TokenType::Const;
  keywords["let"] = TokenType::Let;
  keywords["func"] = TokenType::Func;
  keywords["if"] = TokenType::If;
  keywords["while"] = TokenType::While;
  keywords["return"] = TokenType::Return;
}
