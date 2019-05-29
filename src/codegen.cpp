#include "./codegen.hpp"
#include "./error.hpp"
#include <cassert>
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
  case Decl::Kind::Func:
    visitFuncDecl(dynamic_cast<FuncDecl *>(decl));
    break;
  case Decl::Kind::Const:
    visitConstDecl(dynamic_cast<ConstDecl *>(decl));
    break;
  case Decl::Kind::Type:
    visitTypeDecl(dynamic_cast<TypeDecl *>(decl));
    break;
  default:;
  }
}

llvm::Type *CodeGen::getType(Type *type) {
  if (type->kind == Type::Kind::Ident) {
    IdentType *ident_type = dynamic_cast<IdentType *>(type);
    return getTypeByName(ident_type->name->name);
  } else if (type->kind == Type::Kind::Struct) {
    StructType *struct_type = dynamic_cast<StructType *>(type);

    std::vector<llvm::Type *> fields;
    for (const auto &field : struct_type->fields) {
      fields.push_back(getType(field.type));
    }

    return llvm::StructType::get(context, fields);
  } else if (type->kind == Type::Kind::Array) {
    return getArrayType(dynamic_cast<ArrayType *>(type));
  } else {
    return nullptr;
  }
}

llvm::Type *CodeGen::getTypeByName(const std::string &name) {
  llvm::Type *type = types[name];
  if (type != nullptr) {
    return type;
  }

  type = module->getTypeByName(name);
  if (type != nullptr) {
    return type;
  }

  if (name == "i64") {
    return builder.getInt64Ty();
  } else if (name == "void") {
    return builder.getVoidTy();
  } else {
    error("unknown type " + name + " at getTypeByName");
    throw;
  }
}

llvm::FunctionType *CodeGen::getFuncType(FunctionType *funcType) {
  llvm::Type *funcResult = getType(funcType->result);

  std::vector<llvm::Type *> param_types;
  for (const auto &field : funcType->fields) {
    param_types.push_back(getType(field.type));
  }

  return llvm::FunctionType::get(funcResult, param_types, false);
}

llvm::ArrayType *CodeGen::getArrayType(ArrayType *arrayType) {
  llvm::Type *elementType = getType(arrayType->element);
  unsigned long long length = std::stoull(arrayType->length->value);
  return llvm::ArrayType::get(elementType, length);
}

void CodeGen::visitFuncDecl(FuncDecl *func_decl) {
  auto *funcType = getFuncType(func_decl->func_type);
  auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                      func_decl->name, module);
  auto *bblock = llvm::BasicBlock::Create(context, "entry", func);
  builder.SetInsertPoint(bblock);

  llvm::Function::arg_iterator arg_iter = func->arg_begin();
  for (const auto &field : func_decl->func_type->fields) {
    arg_iter->setName(field.name->name);
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

void CodeGen::visitTypeDecl(TypeDecl *type_decl) {
  if (type_decl->type->kind == Type::Kind::Ident) {
    IdentType *ident_type = dynamic_cast<IdentType *>(type_decl->type);
    llvm::Type *type = getTypeByName(ident_type->name->name);

    this->types[type_decl->name->name] = type;
  } else if (type_decl->type->kind == Type::Kind::Struct) {
    StructType *struct_type = dynamic_cast<StructType *>(type_decl->type);

    std::vector<llvm::Type *> fields;
    for (const auto &field : struct_type->fields) {
      fields.push_back(getType(field.type));
    }

    auto *type = llvm::StructType::get(context, fields);
    type->setName(type_decl->name->name);

    this->structs[type_decl->name->name] = struct_type;
  }
}

void CodeGen::visitBlock(BlockStmt *block) {
  for (Stmt *stmt : block->stmts) {
    visitStmt(stmt);
  }
}

void CodeGen::visitStmt(Stmt *stmt) {
  switch (stmt->kind) {
  case Stmt::Kind::Let:
    visitLetStmt((LetStmt *)stmt);
    break;
  case Stmt::Kind::Return:
    visitReturnStmt((ReturnStmt *)stmt);
    break;
  case Stmt::Kind::If:
    visitIfStmt((IfStmt *)stmt);
    break;
  case Stmt::Kind::Expr:
    visitExprStmt(dynamic_cast<ExprStmt *>(stmt));
    break;
  default:
    error("visitStmt");
  }
}

void CodeGen::visitLetStmt(LetStmt *stmt) {
  llvm::Type *type = nullptr;

  if (stmt->type != nullptr) {
    type = getType(stmt->type);
  }

  llvm::Value *val = nullptr;
  if (stmt->expr != nullptr) {
    val = genExpr(stmt->expr);
    if (type == nullptr) {
      type = val->getType();
    }
  }

  auto *alloca = builder.CreateAlloca(type, 0, stmt->ident->name);
  local_vals.emplace(stmt->ident->name, alloca);

  if (val != nullptr) {
    builder.CreateStore(val, alloca);
  }
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

void CodeGen::visitExprStmt(ExprStmt *stmt) {
  genExpr(stmt->expr);
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
  case Expr::Type::RefExpr:
    return genRefExpr(dynamic_cast<RefExpr *>(expr));
  case Expr::Type::IndexExpr:
    return genIndexExpr(dynamic_cast<IndexExpr *>(expr));
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
  if (expr->op == TokenType::Assign) {
    return genAssignExpr(expr);
  }

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

llvm::Value *CodeGen::genAssignExpr(BinaryExpr *expr) {
  llvm::Value *dist = getRef(expr->lhs);
  llvm::Value *src = genExpr(expr->rhs);
  return builder.CreateStore(src, dist);

  error("unsupported assigner");
  return nullptr;
}

llvm::Value *CodeGen::genRefExpr(RefExpr *expr) {
  llvm::Value *receiver = genExpr(expr->receiver);
  llvm::Type *type = receiver->getType();
  const auto &type_name = type->getStructName();
  StructType *struct_type = this->structs[type_name];

  unsigned int index = struct_type->index(expr->ref->name);
  return builder.CreateExtractValue(receiver, index);
}

llvm::Value *CodeGen::genIndexExpr(IndexExpr *expr) {
  llvm::Value *p = getRefIndexExpr(expr);
  return builder.CreateLoad(p);
}

llvm::Value *CodeGen::getRef(Expr *expr) {
  switch (expr->type) {
  case Expr::Type::Ident:
    return getRefIdent(dynamic_cast<Ident *>(expr));
  case Expr::Type::RefExpr:
    return getRefRefExpr(dynamic_cast<RefExpr *>(expr));
  case Expr::Type::IndexExpr:
    return getRefIndexExpr(dynamic_cast<IndexExpr *>(expr));
  default:
    error("can not get ref of expression");
    return nullptr;
  }
}

llvm::Value *CodeGen::getRefIdent(Ident *ident) {
  auto itr = local_vals.find(ident->name);
  if (itr != local_vals.end()) {
    return itr->second;
  }
  error("undefined value at getRefIdent");
  return nullptr;
}

llvm::Value *CodeGen::getRefRefExpr(RefExpr *expr) {
  llvm::Value *receiver = getRef(expr->receiver);
  llvm::Type *type = receiver->getType()->getPointerElementType();
  const auto &type_name = type->getStructName();
  StructType *struct_type = this->structs[type_name];

  unsigned int index = struct_type->index(expr->ref->name);
  return builder.CreateStructGEP(receiver, index);
}

llvm::Value *CodeGen::getRefIndexExpr(IndexExpr *expr) {
  llvm::Value *receiver = getRef(expr->receiver);
  llvm::Value *index = genExpr(expr->index);

  // llvm::Type *type = receiver->getType();
  // assert(type->isPointerTy());
  // llvm::Type *arr = type->getPointerElementType();
  // assert(arr->isArrayTy());
  // llvm::Type *elementType = arr->getArrayElementType();
  // assert(elementType != nullptr);
  return builder.CreateGEP(receiver, { builder.getInt64(0), index });
}
