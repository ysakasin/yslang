#pragma once

#include <map>
#include <string>

#include "./ast.hpp"
#include "./lexer.hpp"
#include "./token.hpp"

namespace yslang {
class Parser {
public:
  Parser(const std::string &input);
  ~Parser() {}

  Program parse();

  bool has_error() {
    return error_messages.size() != 0;
  }

private:
  enum Precedence {
    LOWEST = 0,
    EQUALS,      // ==
    LESSGREATER, // < or >
    SUM,         // + or -
    PRODUCT,     // * or /
    PREFIX,      // !X or -Y
    CALL,        // myFunction(X)
    INDEX,       // array[index]
    ASSIGN,      // = or :=
  };

private:
  Decl *parse_decl();
  FuncDecl *parse_func_decl();
  FuncType parse_func_type();
  std::vector<Field> parse_params();
  Field parse_param();
  ConstDecl *constDecl();
  void varDecl(std::vector<std::string> *vars);
  BlockStmt *blockStmt();
  Stmt *statement();
  ReturnStmt *returnStmt();
  LetStmt *letStmt();
  IfStmt *parse_if_statement();
  ExprStmt *expr_stmt();

  Expr *parse_expression(Precedence precedence);

  Expr *parseIntegerLiteral();
  Expr *parse_identifier();

  Expr *parse_infix_expression(Expr *left);
  Expr *parse_call_expression(Expr *left);

  std::vector<Expr *> parse_expression_list();

private:
  void next_token() {
    cur_token = std::move(peek_token);
    peek_token = lexer.next();
  }

  void expect(TokenType type) {
    if (cur_token.type != type) {
      std::stringstream ss;
      ss << "expected next token to be " << type << ", got " << cur_token.type;
      error_messages.emplace_back(ss.str());
    }

    next_token();
  }

  bool expect_cur(TokenType type) {
    if (cur_token_is(type)) {
      next_token();
      return true;
    } else {
      // TODO: error msg
      return false;
    }
  }

  bool expect_peek(TokenType type) {
    if (peek_token_is(type)) {
      next_token();
      return true;
    } else {
      peek_error(type);
      return false;
    }
  }

  bool cur_token_is(TokenType type) {
    return cur_token.type == type;
  }

  bool peek_token_is(TokenType type) {
    return peek_token.type == type;
  }

  Precedence cur_precedence() {
    return precedences[cur_token.type];
  }

  Precedence peek_precedence() {
    return precedences[peek_token.type];
  }

  void peek_error(TokenType type) {}

  void append_error(const std::string &message) {
    error_messages.emplace_back(message);
  }

private:
  Lexer lexer;

  Token cur_token;
  Token peek_token;

  using prefix_parse = std::function<Expr *(Parser *)>;
  using infix_parse = std::function<Expr *(Parser *, Expr *)>;
  std::map<TokenType, prefix_parse> prefix_parse_functions;
  std::map<TokenType, infix_parse> infix_parse_functions;
  std::map<TokenType, Precedence> precedences;

public:
  std::vector<std::string> error_messages;
};
} // namespace yslang
