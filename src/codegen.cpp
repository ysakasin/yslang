#include "./codegen.hpp"
#include "./error.hpp"
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/ValueSymbolTable.h>

using namespace yslang;

CodeGen::CodeGen()
    : context(), module(new llvm::Module("top", context)), builder(context) {}

void CodeGen::generate(Program *program) {
  visitProgram(program);
}

void CodeGen::visitProgram(Program *program) {
  for (Decl *decl : program->decls) {
    visitDecl(decl);
  }
}

void CodeGen::visitDecl(Decl *decl) {
  switch (decl->type) {
  case Decl::Type::Func:
    visitFuncDecl((FuncDecl *)decl);
    break;
  case Decl::Type::Const:
    visitConstDecl(dynamic_cast<ConstDecl *>(decl));
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

llvm::FunctionType *CodeGen::getFuncType(const FunctionType &funcType) {
  std::vector<llvm::Type *> param_types;
  llvm::Type *funcResult;
  funcResult = getTypeByName(funcType.result->name);

  for (const auto &fields : funcType.fields) {
    param_types.push_back(getTypeByName(fields.type.name));
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
  for (const auto &field : func_decl->func_type.fields) {
    arg_iter->setName(field.name.name);
    ++arg_iter;
  }

  curFunc = func;
  visitBlock(func_decl->body);
  builder.CreateRet(builder.getInt64(0));
  curFunc = nullptr;
}

void CodeGen::visitConstDecl(ConstDecl *const_decl) {
  auto *val = genExpr(const_decl->expr);
  auto *constant = llvm::dyn_cast<llvm::Constant>(val);
  auto *g =
      new llvm::GlobalVariable(*module, constant->getType(), true,
                               llvm::GlobalValue::LinkageTypes::PrivateLinkage,
                               constant, const_decl->name);
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
  case Stmt::Type::If:
    visitIfStmt((IfStmt *)stmt);
    break;
  // case Stmt::Type::ExprStmt:
  default:
    error("visitStmt");
  }
}

void CodeGen::visitLetStmt(LetStmt *stmt) {
  auto *val = genExpr(stmt->expr);
  auto *alloca = builder.CreateAlloca(val->getType(), 0, stmt->ident->name);
  local_vals.emplace(stmt->ident->name, alloca);
  builder.CreateStore(val, alloca);
}

void CodeGen::visitReturnStmt(ReturnStmt *stmt) {
  auto *retVal = genExpr(stmt->results[0]);
  builder.CreateRet(retVal);

  auto *dummy = llvm::BasicBlock::Create(context, "dummy");
  builder.SetInsertPoint(dummy);
}

void CodeGen::visitIfStmt(IfStmt *stmt) {
  auto *cond = genExpr(stmt->cond);

  auto *then_block = llvm::BasicBlock::Create(context, "if.then", curFunc);
  auto *else_block = llvm::BasicBlock::Create(context, "if.else", curFunc);
  auto *merge_block = llvm::BasicBlock::Create(context, "if.merge");

  builder.CreateCondBr(cond, then_block, else_block);

  builder.SetInsertPoint(then_block);
  visitBlock(dynamic_cast<BlockStmt *>(stmt->then_block));
  builder.CreateBr(merge_block);
  then_block = builder.GetInsertBlock();

  builder.SetInsertPoint(else_block);
  if (stmt->else_block != nullptr) {
    visitBlock(dynamic_cast<BlockStmt *>(stmt->else_block));
  }
  builder.CreateBr(merge_block);
  else_block = builder.GetInsertBlock();

  curFunc->getBasicBlockList().push_back(merge_block);
  builder.SetInsertPoint(merge_block);
}

llvm::Value *CodeGen::genExpr(Expr *expr) {
  switch (expr->type) {
  case Expr::Type::Ident:
    return genIdent((Ident *)expr);
  case Expr::Type::BasicLit:
    return genBasicLit((BasicLit *)expr);
  case Expr::Type::CallExpr:
    return genCallExpr((CallExpr *)expr);
  case Expr::Type::BinaryExpr:
    return genBinaryExpr((BinaryExpr *)expr);
  default:
    error("unknown expression at genExpr");
    return nullptr;
  }
}

llvm::Value *CodeGen::genIdent(Ident *ident) {
  auto itr = local_vals.find(ident->name);
  if (itr != local_vals.end()) {
    return builder.CreateLoad(itr->second);
  }

  auto *vs_table = curFunc->getValueSymbolTable();
  llvm::Value *v = vs_table->lookup(ident->name);
  if (v != nullptr) {
    return v;
  }

  v = module->getNamedValue(ident->name);
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

llvm::Value *CodeGen::genBinaryExpr(BinaryExpr *expr) {
  llvm::Value *lhs = genExpr(expr->lhs);
  llvm::Value *rhs = genExpr(expr->rhs);
  switch (expr->op) {
  case TokenType::Plus:
    return builder.CreateAdd(lhs, rhs);
  case TokenType::Minus:
    return builder.CreateSub(lhs, rhs);
  case TokenType::LessEqual:
    return builder.CreateICmpSLE(lhs, rhs);
  default:
    std::stringstream ss;
    ss << "not support binop " << expr->op;
    error(ss.str());
    throw;
  }
}
