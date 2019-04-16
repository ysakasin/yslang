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
  IfStmt *ifStmt();

  Expr *expr(int precedence);
  Expr *binaryExpr();
  Expr *term();
  Expr *factor();
  Expr *factorIdent();
  Expr *parseOperand();
  CallExpr *parseCallExpr(Expr *callee);

  Expr *parseIntegerLiteral();

  // llvm::Value *condition();
  // llvm::Value *expression();
  // llvm::Value *term();
  // llvm::Value *factor();
  // llvm::Value *factorIdent();

private:
  void next_token() {
    cur_token = std::move(peek_token);
    peek_token = lexer.next();
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

  int peek_precedence() {
    return precedences[peek_token.type];
  }

  void peek_error(TokenType type) {}

private:
  Lexer lexer;

  Token cur_token;
  Token peek_token;

  using prefix_parse = std::function<Expr *(Parser *)>;
  using infix_parse = std::function<Expr *(Parser *, Expr *)>;
  std::map<TokenType, prefix_parse> prefix_parse_functions;
  std::map<TokenType, infix_parse> infix_parse_functions;
  std::map<TokenType, int> precedences;
};
} // namespace yslang
