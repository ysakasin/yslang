#include "./ast.hpp"
#include <sstream>

using namespace yslang;

// -------------------- //
// Expr
// -------------------- //

json BinaryExpr::toJson() const {
  json j;
  std::stringstream ss;

  ss << op;

  j["kind"] = "BinaryExpr";
  j["lhs"] = lhs->toJson();
  j["op"] = ss.str();
  j["rhs"] = rhs->toJson();
  return j;
}

json Ident::toJson() const {
  json j;
  j["kind"] = "Ident";
  j["name"] = name;
  return j;
}

json CallExpr::toJson() const {
  json j;
  j["kind"] = "CallExpr";
  j["func"] = func->toJson();
  j["args"] = json::array();
  for (const Expr *arg : args) {
    j["args"].push_back(arg->toJson());
  }
  return j;
}

json RefExpr::toJson() const {
  json j;
  j["kind"] = "RefExpr";
  j["receiver"] = receiver->toJson();
  j["ref"] = ref->toJson();
  return j;
}

json BasicLit::toJson() const {
  json j;
  std::stringstream ss;

  ss << kind;

  j["kind"] = "BasicLit";
  j["kind"] = ss.str();
  j["value"] = value;
  return j;
}

// -------------------- //
// Type
// -------------------- //

json Field::toJson() const {
  json j;
  j["kind"] = "Field";
  j["name"] = name->toJson();
  j["type"] = type->toJson();
  return j;
}

json IdentType::toJson() const {
  json j;
  j["kind"] = "IdentType";
  j["name"] = name->toJson();
  return j;
}

json StructType::toJson() const {
  json j;
  j["kind"] = "StructType";
  j["fields"] = json::array();
  for (const auto &field : fields) {
    j["fields"].push_back(field.toJson());
  }
  return j;
}

json FunctionType::toJson() const {
  json j;
  j["kind"] = "FunctionType";
  j["result"] = result->toJson();
  j["fields"] = json::array();
  for (const auto &field : fields) {
    j["fields"].push_back(field.toJson());
  }
  return j;
}

// -------------------- //
// Stmt
// -------------------- //

json BlockStmt::toJson() const {
  json j;
  j["kind"] = "BlockStmt";
  j["stmts"] = json::array();
  for (const Stmt *stmt : stmts) {
    j["stmts"].push_back(stmt->toJson());
  }
  return j;
}

json LetStmt::toJson() const {
  json j;
  j["kind"] = "LetStmt";
  j["ident"] = ident->toJson();
  if (type != nullptr) {
    j["type"] = type->toJson();
  }
  if (expr != nullptr) {
    j["expr"] = expr->toJson();
  }
  return j;
}

json ReturnStmt::toJson() const {
  json j;
  j["kind"] = "ReturnStmt";
  j["results"].init_array();
  for (const Expr *expr : results) {
    j["results"].push_back(expr->toJson());
  }
  return j;
}

json IfStmt::toJson() const {
  json j;
  j["kind"] = "IfStmt";
  j["cond"] = cond->toJson();
  j["then_block"] = then_block->toJson();
  if (else_block != nullptr) {
    j["else_block"] = else_block->toJson();
  }
  return j;
}

// -------------------- //
// Decl
// -------------------- //

json FuncDecl::toJson() const {
  json j;
  j["kind"] = "FuncDecl";
  j["name"] = name;
  j["type"] = func_type->toJson();
  j["body"] = body->toJson();
  return j;
}

json ConstDecl::toJson() const {
  json j;
  j["kind"] = "ConstDecl";
  j["name"] = name;
  j["expr"] = expr->toJson();
  return j;
}

json ImportDecl::toJson() const {
  json j;
  j["kind"] = "ImportDecl";
  j["package"] = package->toJson();
  return j;
}

json TypeDecl::toJson() const {
  json j;
  j["kind"] = "TypeDecl";
  j["name"] = name->toJson();
  j["type"] = type->toJson();
  return j;
}

json Program::toJson() const {
  json j;
  j["kind"] = "Program";
  j["path"] = path;
  j["decls"] = json::array();
  for (const Decl *decl : decls) {
    j["decls"].push_back(decl->toJson());
  }
  return j;
}

json ExprStmt::toJson() const {
  json j;
  j["kind"] = "ExprStmt";
  j["expr"] = expr->toJson();
  return j;
}
