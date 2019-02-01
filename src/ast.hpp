#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "./token.hpp"

namespace yslang {
static std::string indent(int depth) { return std::string(depth * 3, ' '); }
class Node {
public:
  virtual void print(std::ostream &out, int depth) = 0;
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
  std::string name;
  void print(std::ostream &out, int d) {
    out << indent(d) << "Ident {" << name << "}" << std::endl;
  }
};
class CallExpr : public Expr {
public:
  CallExpr() : Expr(Expr::Type::CallExpr) {}
  Expr *func;
  std::vector<Expr *> args;
  void print(std::ostream &out, int d) {
    out << indent(d) << "CallExpr { omited }" << std::endl;
  }
};
class BasicLit : public Expr {
public:
  BasicLit() : Expr(Expr::Type::BasicLit) {}

public:
  TokenType kind;
  std::string value;
  void print(std::ostream &out, int d) {
    out << indent(d) << "BasicLit {" << std::endl;
    out << indent(d) << "  kind: " << kind << std::endl;
    out << indent(d) << "  value: " << value << std::endl;
    out << indent(d) << "}" << std::endl;
  }
};
class BlockStmt : public Stmt {
public:
  BlockStmt() : Stmt(Stmt::Type::Block) {}

public:
  std::vector<Stmt *> stmts;
  void print(std::ostream &out, int d) {
    out << indent(d) << "BlockStmt {" << std::endl;
    for (Stmt *stmt : stmts) {
      stmt->print(out, d + 1);
    }
    out << indent(d) << "}" << std::endl;
  }
};
class LetStmt : public Stmt {
public:
  LetStmt() : Stmt(Stmt::Type::Let) {}

public:
  std::string ident;
  Expr *expr;
  void print(std::ostream &out, int d) {
    out << indent(d) << "LetStmt {" << std::endl;
    out << indent(d) << "  ident: " << ident << std::endl;
    out << indent(d) << "  expr:" << std::endl;
    expr->print(out, d + 1);
    out << indent(d) << "}" << std::endl;
  }
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt() : Stmt(Stmt::Type::Return) {}

public:
  std::vector<Expr *> results;
  void print(std::ostream &out, int d) {
    out << indent(d) << "ReturnStmt {" << std::endl;
    for (Expr *expr : results) {
      expr->print(out, d + 1);
    }
    out << indent(d) << "}" << std::endl;
  }
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

public:
  std::string name;
  FuncType func_type;
  BlockStmt *body;
  void print(std::ostream &out, int d) {
    out << indent(d) << "FuncStmt {" << std::endl;
    out << indent(d) << "  name: " << name << std::endl;
    out << indent(d) << "  body:" << std::endl;
    body->print(out, d + 1);
    out << indent(d) << "}" << std::endl;
  }
};
class File : public Node {
public:
  std::string path;
  std::vector<Decl *> decls;
  void print(std::ostream &out, int d = 0) {
    out << indent(d) << "File {" << std::endl;
    out << indent(d) << "  path: " << path << std::endl;
    out << indent(d) << "  decls:" << std::endl;
    for (Decl *decl : decls) {
      decl->print(out, d + 1);
    }
    out << indent(d) << "}" << std::endl;
  }
};

} // namespace yslang
