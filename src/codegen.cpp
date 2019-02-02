#include "./codegen.hpp"
#include "./error.hpp"
#include <llvm/IR/ValueSymbolTable.h>

using namespace yslang;

CodeGen::CodeGen()
    : context(), module(new llvm::Module("top", context)), builder(context) {}

void CodeGen::generate(File *file) { visitFile(file); }

void CodeGen::visitFile(File *file) {
  for (Decl *decl : file->decls) {
    visitDecl(decl);
  }
}

void CodeGen::visitDecl(Decl *decl) {
  switch (decl->type) {
  case Decl::Type::Func:
    visitFuncDecl((FuncDecl *)decl);
    break;
  default:;
  }
}

llvm::Type *CodeGen::getTypeByName(const std::string &name) {
  if (name == "i64") {
    return builder.getInt64Ty();
  } else if (name == "void") {
    return builder.getVoidTy();
  } else {
    error("unknown type " + name + " at getTypeByName");
    throw;
  }
}

llvm::FunctionType *CodeGen::getFuncType(const FuncType &funcType) {
  std::vector<llvm::Type *> param_types;
  llvm::Type *funcResult;
  if (funcType.results.size() == 0) {
    funcResult = builder.getVoidTy();
  } else {
    funcResult = getTypeByName(funcType.results[0]);
  }

  for (const auto &arg : funcType.args) {
    param_types.push_back(getTypeByName(arg.type));
  }

  return llvm::FunctionType::get(funcResult, param_types, false);
}

void CodeGen::visitFuncDecl(FuncDecl *func_decl) {
  auto *funcType = getFuncType(func_decl->func_type);
  auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      func_decl->name, module);
  auto *bblock = llvm::BasicBlock::Create(context, "entry", func);
  builder.SetInsertPoint(bblock);

  llvm::Function::arg_iterator arg_iter = func->arg_begin();
  for (const auto &arg : func_decl->func_type.args) {
    arg_iter->setName(arg.name);
    ++arg_iter;
  }

  curFunc = func;
  visitBlock(func_decl->body);
  curFunc = nullptr;
}

void CodeGen::visitBlock(BlockStmt *block) {
  for (Stmt *stmt : block->stmts) {
    visitStmt(stmt);
  }
}

void CodeGen::visitStmt(Stmt *stmt) {
  switch (stmt->type) {
  case Stmt::Type::Let:
    visitLetStmt((LetStmt *)stmt);
    break;
  case Stmt::Type::Return:
    visitReturnStmt((ReturnStmt *)stmt);
    break;
  default:;
  }
}

void CodeGen::visitLetStmt(LetStmt *stmt) {
  auto *val = genExpr(stmt->expr);
  auto *alloca = builder.CreateAlloca(val->getType(), 0, stmt->ident);
  local_vals.emplace(stmt->ident, alloca);
  builder.CreateStore(val, alloca);
}

void CodeGen::visitReturnStmt(ReturnStmt *stmt) {
  auto *retVal = genExpr(stmt->results[0]);
  builder.CreateRet(retVal);
}

llvm::Value *CodeGen::genExpr(Expr *expr) {
  switch (expr->type) {
  case Expr::Type::Ident:
    return genIdent((Ident *)expr);
  case Expr::Type::BasicLit:
    return genBasicLit((BasicLit *)expr);
  case Expr::Type::CallExpr:
    return genCallExpr((CallExpr *)expr);
  default:
    error("unknown expression at genExpr");
  }
}

llvm::Value *CodeGen::genIdent(Ident *ident) {
  auto itr = local_vals.find(ident->name);
  if (itr != local_vals.end()) {
    return builder.CreateLoad(itr->second);
  }

  auto *vs_table = curFunc->getValueSymbolTable();
  llvm::Value *v = vs_table->lookup(ident->name);
  if (v == nullptr) {
    error("undefined ident " + ident->name + " at genIdent");
  }
  return v;
}

llvm::Value *CodeGen::genBasicLit(BasicLit *lit) {
  switch (lit->kind) {
  case TokenType::Integer:
    return builder.getInt64(std::stoll(lit->value));
  default:
    error("unsupported literal at genBasicLit");
    throw;
  }
}

llvm::Value *CodeGen::genCallExpr(CallExpr *callExpr) {
  llvm::Function *func;
  switch (callExpr->func->type) {
  case Expr::Type::Ident:
    func = module->getFunction(((Ident *)callExpr->func)->name);
    break;
  default:
    error("unsupported expr at genCallExpr");
    throw;
  }

  std::vector<llvm::Value *> args;
  for (Expr *expr : callExpr->args) {
    args.push_back(genExpr(expr));
  }
  return builder.CreateCall(func, args);
}
