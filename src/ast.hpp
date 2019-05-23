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
  enum class Kind { Const, Func, Import, Type };
  Decl(Kind type) : type(type){};

public:
  const Kind type;
};

class Stmt : public Node {
public:
  enum class Kind { Block, Return, Let, If, Expr };
  Stmt(Kind kind) : kind(kind){};

public:
  const Kind kind;
};

class Expr : public Node {
public:
  enum class Type { BasicLit, Ident, CallExpr, BinaryExpr, RefExpr };
  Expr(Type type) : type(type){};

public:
  const Type type;
};

class Type : public Node {
public:
  enum class Kind { Ident, Struct, Function };
  Type(Kind kind) : kind(kind){};

public:
  const Kind kind;
};

class Program : public Node {
public:
  json toJson() const;

public:
  std::string path;
  std::vector<Decl *> decls;
};

// -------------------- //
// Expr
// -------------------- //
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

// -------------------- //
// Type
// -------------------- //

class Field {
public:
  json toJson() const;

public:
  Ident *name;
  Type *type;
};

class IdentType : public Type {
public:
  IdentType() : Type(Type::Kind::Ident) {}
  json toJson() const;

public:
  Ident *name;
};

class StructType : public Type {
public:
  StructType() : Type(Type::Kind::Struct) {}
  json toJson() const;

public:
  std::vector<Field> fields;
};

class FunctionType : public Type {
public:
  FunctionType() : Type(Type::Kind::Function) {}
  json toJson() const;

public:
  Type *result;
  std::vector<Field> fields;
};

// -------------------- //
// Stmt
// -------------------- //

class ExprStmt : public Stmt {
public:
  ExprStmt() : Stmt(Stmt::Kind::Expr) {}
  json toJson() const;

public:
  Expr *expr;
};

class BlockStmt : public Stmt {
public:
  BlockStmt() : Stmt(Stmt::Kind::Block) {}
  json toJson() const;

public:
  std::vector<Stmt *> stmts;
};

class LetStmt : public Stmt {
public:
  LetStmt() : Stmt(Stmt::Kind::Let) {}
  json toJson() const;

public:
  Ident *ident;
  Type *type;
  Expr *expr;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt() : Stmt(Stmt::Kind::Return) {}
  json toJson() const;

public:
  std::vector<Expr *> results;
};

class IfStmt : public Stmt {
public:
  IfStmt() : Stmt(Stmt::Kind::If) {}
  json toJson() const;

public:
  Expr *cond;
  Stmt *then_block;
  Stmt *else_block;
};

// -------------------- //
// Decl
// -------------------- //

class FuncDecl : public Decl {
public:
  FuncDecl() : Decl(Decl::Kind::Func) {}
  json toJson() const;

public:
  std::string name;
  FunctionType *func_type;
  BlockStmt *body;
};

class ConstDecl : public Decl {
public:
  ConstDecl() : Decl(Decl::Kind::Const) {}
  json toJson() const;

public:
  std::string name;
  Expr *expr;
};

class ImportDecl : public Decl {
public:
  ImportDecl() : Decl(Decl::Kind::Import) {}
  json toJson() const;

public:
  Ident *package;
};

class TypeDecl : public Decl {
public:
  TypeDecl() : Decl(Decl::Kind::Type) {}
  json toJson() const;

public:
  Ident *name;
  Type *type;
};

} // namespace yslang
