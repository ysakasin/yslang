#include "./ast.hpp"
#include <sstream>

using namespace yslang;
using json = nlohmann::json;

json BinaryExpr::toJson() const {
  json j;
  std::stringstream ss;

  ss << op;

  j["#ast type"] = "Ident";
  j["0 lhs"] = lhs->toJson();
  j["1 op"] = ss.str();
  j["2 rhs"] = rhs->toJson();
  return j;
}

json Ident::toJson() const {
  json j;
  j["#ast type"] = "Ident";
  j["name"] = name;
  return j;
}

json CallExpr::toJson() const {
  json j;
  j["#ast type"] = "CallExpr";
  j["func"] = func->toJson();
  j["args"] = json::array();
  for (const Expr *arg : args) {
    j["args"].push_back(arg->toJson());
  }
  return j;
}

json BasicLit::toJson() const {
  json j;
  std::stringstream ss;

  ss << kind;

  j["#ast type"] = "BasicLit";
  j["kind"] = ss.str();
  j["value"] = value;
  return j;
}

json BlockStmt::toJson() const {
  json j;
  j["#ast type"] = "BlockStmt";
  j["stmts"] = json::array();
  for (const Stmt *stmt : stmts) {
    j["stmts"].push_back(stmt->toJson());
  }
  return j;
}

json LetStmt::toJson() const {
  json j;
  j["#ast type"] = "LetStmt";
  j["ident"] = ident;
  j["expr"] = expr->toJson();
  return j;
}

json ReturnStmt::toJson() const {
  json j;
  j["#ast type"] = "ReturnStmt";
  j["results"] = json::array();
  for (const Expr *expr : results) {
    j["results"].push_back(expr->toJson());
  }
  return j;
}

json IfStmt::toJson() const {
  json j;
  j["#ast type"] = "IfStmt";
  j["cond"] = cond->toJson();
  j["then_block"] = then_block->toJson();
  if (else_block != nullptr) {
    j["else_block"] = else_block->toJson();
  } else {
    j["else_block"] = "nullptr";
  }
  return j;
}

json FuncType::toJson() const {
  json j;
  j["results"] = json(results);
  j["args"] = json::array();
  for (const auto &arg : args) {
    j["args"].push_back({{"name", arg.name}, {"type", arg.type}});
  }
  return j;
}

json FuncDecl::toJson() const {
  json j;
  j["#ast type"] = "FuncDecl";
  j["name"] = name;
  j["body"] = body->toJson();
  j["type"] = func_type.toJson();
  return j;
}

json ConstDecl::toJson() const {
  json j;
  j["#ast type"] = "ConstDecl";
  j["name"] = name;
  j["expr"] = expr->toJson();
  return j;
}

json File::toJson() const {
  json j;
  j["#ast type"] = "File";
  j["path"] = path;
  j["decls"] = json::array();
  for (const Decl *decl : decls) {
    j["decls"].push_back(decl->toJson());
  }
  return j;
}
