#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "../third_party/json.hpp"
#include "./token.hpp"

namespace yslang {
class Node {
public:
  virtual nlohmann::json toJson() const = 0;
};

class Decl : public Node {
public:
  enum class Type { Func };
  Decl(Type type) : type(type){};

public:
  const Type type;
};

class Stmt : public Node {
public:
  enum class Type { Block, Return, Let };
  Stmt(Type type) : type(type){};

public:
  const Type type;
};

class Expr : public Node {
public:
  enum class Type { BasicLit, Ident, CallExpr };
  Expr(Type type) : type(type){};

public:
  const Type type;
};

class Ident : public Expr {
public:
  Ident() : Expr(Expr::Type::Ident) {}
  nlohmann::json toJson() const;

public:
  std::string name;
};

class CallExpr : public Expr {
public:
  CallExpr() : Expr(Expr::Type::CallExpr) {}
  nlohmann::json toJson() const;

public:
  Expr *func;
  std::vector<Expr *> args;
};

class BasicLit : public Expr {
public:
  BasicLit() : Expr(Expr::Type::BasicLit) {}
  nlohmann::json toJson() const;

public:
  TokenType kind;
  std::string value;
};

class BlockStmt : public Stmt {
public:
  BlockStmt() : Stmt(Stmt::Type::Block) {}
  nlohmann::json toJson() const;

public:
  std::vector<Stmt *> stmts;
};

class LetStmt : public Stmt {
public:
  LetStmt() : Stmt(Stmt::Type::Let) {}
  nlohmann::json toJson() const;

public:
  std::string ident;
  Expr *expr;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt() : Stmt(Stmt::Type::Return) {}
  nlohmann::json toJson() const;

public:
  std::vector<Expr *> results;
};

class FuncType {
public:
  class Arg {
  public:
    std::string name;
    std::string type;
  };
  std::vector<std::string> results;
  std::vector<Arg> args;
};

class FuncDecl : public Decl {
public:
  FuncDecl() : Decl(Decl::Type::Func) {}
  nlohmann::json toJson() const;

public:
  std::string name;
  FuncType func_type;
  BlockStmt *body;
};

class File : public Node {
public:
  nlohmann::json toJson() const;

public:
  std::string path;
  std::vector<Decl *> decls;
};

} // namespace yslang
