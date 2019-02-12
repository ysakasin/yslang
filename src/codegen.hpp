#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <map>

#include "./ast.hpp"

namespace yslang {
class CodeGen {
public:
  CodeGen();
  void generate(File *file);
  llvm::Module *getModule() { return module; }

private:
  void visitFile(File *file);
  void visitDecl(Decl *decl);
  void visitFuncDecl(FuncDecl *func);
  void visitConstDecl(ConstDecl *constDecl);
  void visitBlock(BlockStmt *block);
  void visitStmt(Stmt *stmt);
  void visitLetStmt(LetStmt *stmt);
  void visitReturnStmt(ReturnStmt *stmt);
  void visitIfStmt(IfStmt *stmt);

  llvm::Value *genExpr(Expr *expr);
  llvm::Value *genIdent(Ident *ident);
  llvm::Value *genBasicLit(BasicLit *lit);
  llvm::Value *genCallExpr(CallExpr *lit);
  llvm::Value *genBinaryExpr(BinaryExpr *expr);

  llvm::FunctionType *getFuncType(const FuncType &type);
  llvm::Type *getTypeByName(const std::string &name);

private:
  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> builder;

  llvm::Function *curFunc;
  std::map<std::string, llvm::AllocaInst *> local_vals;
};
} // namespace yslang
