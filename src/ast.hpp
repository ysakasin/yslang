#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "./json.hpp"
#include "./token.hpp"

namespace yslang {
class Node {
public:
  virtual json toJson() const = 0;
};

class Decl : public Node {
public:
  enum class Type { Const, Func, Import };
  Decl(Type type) : type(type){};

public:
  const Type type;
};

class Stmt : public Node {
public:
  enum class Type { Block, Return, Let, If, Expr };
  Stmt(Type type) : type(type){};

public:
  const Type type;
};

class Expr : public Node {
public:
  enum class Type { BasicLit, Ident, CallExpr, BinaryExpr, RefExpr };
  Expr(Type type) : type(type){};

public:
  const Type type;
};

class BinaryExpr : public Expr {
public:
  BinaryExpr() : Expr(Expr::Type::BinaryExpr) {}
  json toJson() const;

public:
  Expr *lhs;
  TokenType op;
  Expr *rhs;
};

class Ident : public Expr {
public:
  Ident() : Expr(Expr::Type::Ident) {}
  Ident(const Ident &ident) : Expr(Expr::Type::Ident), name(ident.name) {}

  Ident &operator=(const Ident &ident) {
    name = ident.name;
    return *this;
  }
  json toJson() const;

public:
  std::string name;
};

class CallExpr : public Expr {
public:
  CallExpr() : Expr(Expr::Type::CallExpr) {}
  json toJson() const;

public:
  Expr *func;
  std::vector<Expr *> args;
};

class RefExpr : public Expr {
public:
  RefExpr() : Expr(Expr::Type::RefExpr) {}
  json toJson() const;

public:
  Expr *receiver;
  Ident *ref;
};

class BasicLit : public Expr {
public:
  BasicLit() : Expr(Expr::Type::BasicLit) {}
  json toJson() const;

public:
  TokenType kind;
  std::string value;
};

class ExprStmt : public Stmt {
public:
  ExprStmt() : Stmt(Stmt::Type::Expr) {}
  json toJson() const;

public:
  Expr *expr;
};

class BlockStmt : public Stmt {
public:
  BlockStmt() : Stmt(Stmt::Type::Block) {}
  json toJson() const;

public:
  std::vector<Stmt *> stmts;
};

class LetStmt : public Stmt {
public:
  LetStmt() : Stmt(Stmt::Type::Let) {}
  json toJson() const;

public:
  Ident *ident;
  Expr *expr;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt() : Stmt(Stmt::Type::Return) {}
  json toJson() const;

public:
  std::vector<Expr *> results;
};

class IfStmt : public Stmt {
public:
  IfStmt() : Stmt(Stmt::Type::If) {}
  json toJson() const;

public:
  Expr *cond;
  Stmt *then_block;
  Stmt *else_block;
};

class Field {
public:
  Ident name;
  Ident type;
};

class FuncType {
public:
  json toJson() const;

public:
  Ident result;
  std::vector<Field> fields;
};

class FuncDecl : public Decl {
public:
  FuncDecl() : Decl(Decl::Type::Func) {}
  json toJson() const;

public:
  std::string name;
  FuncType func_type;
  BlockStmt *body;
};

class ConstDecl : public Decl {
public:
  ConstDecl() : Decl(Decl::Type::Const) {}
  json toJson() const;

public:
  std::string name;
  Expr *expr;
};

class ImportDecl : public Decl {
public:
  ImportDecl() : Decl(Decl::Type::Import) {}
  json toJson() const;

public:
  Ident *package;
};

class Program : public Node {
public:
  json toJson() const;

public:
  std::string path;
  std::vector<Decl *> decls;
};

} // namespace yslang
