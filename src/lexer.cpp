#include "./lexer.hpp"
#include "./token.hpp"

using namespace yslang;

std::map<std::string, TokenType> Lexer::keywords;

bool Lexer::try_readc(char c) {
  if (c == peekc()) {
    head++;
    return true;
  }
  return false;
}

Lexer::Lexer(const std::string &path) : path(path) {
  Lexer::init_keywords();
  std::ifstream ifs(path);
  if (ifs.fail()) {
    throw "Can not open " + path;
  }
  std::istreambuf_iterator<char> it(ifs);
  std::istreambuf_iterator<char> last;
  source_program = std::move(std::string(it, last));
}

Token Lexer::next() {
  skip_blank();
  switch (peekc()) {
  case '0' ... '9':
    return read_number();
  case 'a' ... 'z':
  case 'A' ... 'Z':
  case '_':
    return read_ident();
  case '"':
    return read_string_lit();
  case '\0':
    return Token(TokenType::TEOF);
  case '+':
    readc();
    return Token(TokenType::Plus);
  case '-':
    readc();
    return Token(TokenType::Minus);
  case '*':
    readc();
    return Token(TokenType::Mul);
  case '/':
    readc();
    return Token(TokenType::Div);
  case '=':
    readc();
    if (try_readc('=')) {
      return Token(TokenType::Equal);
    } else {
      return Token(TokenType::Assign);
    }
  case '<':
    readc();
    if (try_readc('=')) {
      return Token(TokenType::LessEqual);
    } else if (try_readc('>')) {
      return Token(TokenType::NotEqual);
    } else {
      return Token(TokenType::Less);
    }
  case '>':
    readc();
    if (try_readc('=')) {
      return Token(TokenType::GreaterEqual);
    } else {
      return Token(TokenType::Greater);
    }
  case '(':
    readc();
    return Token(TokenType::ParenL);
  case ')':
    readc();
    return Token(TokenType::ParenR);
  case '{':
    readc();
    return Token(TokenType::BraceL);
  case '}':
    readc();
    return Token(TokenType::BraceR);
  case '.':
    readc();
    return Token(TokenType::Dot);
  case ':':
    readc();
    if (try_readc('=')) {
      return Token(TokenType::Assign);
    } else {
      return Token(TokenType::Colon);
    }
  case ';':
    readc();
    return Token(TokenType::Semicolon);
  case ',':
    readc();
    return Token(TokenType::Comma);
  default:
    throw "invalid char";
  }
}

Token Lexer::take(TokenType type) {
  Token t = next();
  if (t.type != type) {
    throw "unexpected token";
  }
  return std::move(t);
}

void Lexer::print_all() {
  while (true) {
    Token token = next();
    std::cout << token << std::endl;
    if (token.type == TokenType::TEOF) {
      break;
    }
  }
}

void Lexer::skip_blank() {
  // skip ' ', '\n', \t', '\v', '\f', '\r'
  while (isspace(peekc())) {
    readc();
  }
}

Token Lexer::read_number() {
  std::string digit;
  while (isdigit(peekc())) {
    digit.push_back(readc());
  }
  return Token(TokenType::Integer, std::move(digit));
}

bool is_ident_piece(char c) { return isalnum(c) || c == '_'; }

Token Lexer::read_ident() {
  std::string ident;
  while (is_ident_piece(peekc())) {
    ident.push_back(readc());
  }

  auto itr = keywords.find(ident);
  if (itr == keywords.end()) {
    return Token(TokenType::Ident, std::move(ident));
  } else {
    return Token(itr->second);
  }
}

Token Lexer::read_string_lit() {
  readc(); // take '"'

  std::string str;
  char c;
  while ((c = readc()) != '"') {
    str.push_back(c);
  }
  return Token(TokenType::String, std::move(str));
}

void Lexer::print_head() { std::cout << "head: " << head << std::endl; }

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
