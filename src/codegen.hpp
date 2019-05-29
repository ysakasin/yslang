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
  void generate(Program *program);
  llvm::Module *getModule() {
    return module;
  }

private:
  void visitProgram(Program *program);
  void visitDecl(Decl *decl);
  void visitFuncDecl(FuncDecl *func);
  void visitConstDecl(ConstDecl *constDecl);
  void visitTypeDecl(TypeDecl *typeDecl);
  void visitBlock(BlockStmt *block);

  // Statement
  void visitStmt(Stmt *stmt);
  void visitLetStmt(LetStmt *stmt);
  void visitReturnStmt(ReturnStmt *stmt);
  void visitIfStmt(IfStmt *stmt);
  void visitExprStmt(ExprStmt *stmt);

  llvm::Value *genExpr(Expr *expr);
  llvm::Value *genIdent(Ident *ident);
  llvm::Value *genBasicLit(BasicLit *lit);
  llvm::Value *genCallExpr(CallExpr *lit);
  llvm::Value *genBinaryExpr(BinaryExpr *expr);
  llvm::Value *genAssignExpr(BinaryExpr *expr);
  llvm::Value *genRefExpr(RefExpr *expr);
  llvm::Value *genIndexExpr(IndexExpr *expr);

  llvm::Value *getRef(Expr *expr);
  llvm::Value *getRefIdent(Ident *ident);
  llvm::Value *getRefRefExpr(RefExpr *ref);
  llvm::Value *getRefIndexExpr(IndexExpr *ref);

  llvm::Type *getType(Type *type);
  llvm::StructType *getStructType(StructType *type);
  llvm::FunctionType *getFuncType(FunctionType *type);
  llvm::ArrayType *getArrayType(ArrayType *type);
  llvm::Type *getTypeByName(const std::string &name);
  void setTypeAlias(const std::string &name, llvm::Type *type);

private:
  llvm::LLVMContext context;
  llvm::Module *module;
  llvm::IRBuilder<> builder;

  llvm::Function *curFunc;
  std::map<std::string, llvm::AllocaInst *> local_vals;
  std::map<std::string, llvm::Type *> types;
  std::map<std::string, StructType *> structs;
};
} // namespace yslang
