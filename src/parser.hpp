#pragma once

#include <string>

#include "./ast.hpp"
#include "./lexer.hpp"
#include "./token.hpp"

namespace yslang {
class Parser {
public:
  Parser(const std::string &path);
  ~Parser() {}

  File *parse();
  void print();

public:
  void toplevel();

  ConstDecl *constDecl();
  void varDecl(std::vector<std::string> *vars);
  FuncDecl *funcDecl();
  BlockStmt *blockStmt();
  Stmt *statement();
  ReturnStmt *returnStmt();
  LetStmt *letStmt();
  IfStmt *ifStmt();

  Expr *expr();
  Expr *binaryExpr();
  Expr *term();
  Expr *factor();
  Expr *factorIdent();
  Expr *parseOperand();
  CallExpr *parseCallExpr(Expr *callee);

  // llvm::Value *condition();
  // llvm::Value *expression();
  // llvm::Value *term();
  // llvm::Value *factor();
  // llvm::Value *factorIdent();

private:
  void nextToken() {
    cur_token = std::move(peek_token);
    peek_token = lexer.next();
  }

  void takeToken(TokenType type) {
    if (cur_token.type != type) {
      throw "takeToken";
      // parseError(type, cur_token.type);
    }
    nextToken();
  }

private:
  File file;

  Lexer lexer;

  Token cur_token;
  Token peek_token;
};
} // namespace yslang
