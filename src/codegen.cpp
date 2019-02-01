#include "./codegen.hpp"

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

llvm::FunctionType *CodeGen::getFuncType(const FuncType &funcType) {
  std::vector<llvm::Type *> param_types;
  llvm::Type *funcResult;
  if (funcType.results.size() == 0) {
    funcResult = builder.getVoidTy();
  } else if (funcType.results[0] == "i64") {
    funcResult = builder.getInt64Ty();
  } else {
    throw "err";
  }
  return llvm::FunctionType::get(funcResult, param_types, false);
}

void CodeGen::visitFuncDecl(FuncDecl *func_decl) {
  auto *funcType = getFuncType(func_decl->func_type);
  auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      func_decl->name, module);
  auto *bblock = llvm::BasicBlock::Create(context, "entry", func);
  builder.SetInsertPoint(bblock);
  visitBlock(func_decl->body);
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
    throw "err: genExpr";
  }
}

llvm::Value *CodeGen::genIdent(Ident *ident) {
  auto itr = local_vals.find(ident->name);
  if (itr == local_vals.end()) {
    throw "err: genIdent";
  }
  return builder.CreateLoad(itr->second);
}

llvm::Value *CodeGen::genBasicLit(BasicLit *lit) {
  switch (lit->kind) {
  case TokenType::Integer:
    return builder.getInt64(std::stoll(lit->value));
  default:
    throw "err";
  }
}

llvm::Value *CodeGen::genCallExpr(CallExpr *callExpr) {
  llvm::Function *func;
  switch (callExpr->func->type) {
  case Expr::Type::Ident:
    func = module->getFunction(((Ident *)callExpr->func)->name);
    return builder.CreateCall(func);
  default:
    throw "err";
  }
}
