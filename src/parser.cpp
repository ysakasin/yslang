#include <string>

#include "./parser.hpp"

using namespace yslang;

Parser::Parser(const std::string &path) : lexer(path) {
  file.path = path;
  cur_token = lexer.next();
  peek_token = lexer.next();
}

void Parser::print() { std::cout << file.toJson().dump(2) << std::endl; }

File *Parser::parse() {
  Decl *decl;
  while (true) {
    switch (cur_token.type) {
    case TokenType::Func:
      decl = funcDecl();
      break;
    default:
      return &file;
    }
    file.decls.push_back(decl);
  }
  return &file;
}

FuncDecl *Parser::funcDecl() {
  takeToken(TokenType::Func);
  if (cur_token.type != TokenType::Ident) {
    // parseError(TokenType::Ident, cur_token.type);
  }
  std::string func_name = std::move(cur_token.str);
  nextToken();

  std::vector<FuncType::Arg> params;
  takeToken(TokenType::ParenL);
  while (true) {
    if (cur_token.type != TokenType::Ident) {
      break;
    }

    std::string name = std::move(cur_token.str);
    nextToken();
    takeToken(TokenType::Colon);

    if (cur_token.type != TokenType::Ident) {
      throw "need type";
    }

    std::string type = std::move(cur_token.str);
    nextToken();

    params.push_back({name, type});
    if (cur_token.type == TokenType::Comma) {
      nextToken();
    } else {
      break;
    }
  }
  takeToken(TokenType::ParenR);

  FuncType func_type;
  if (cur_token.type == TokenType::Ident) {
    func_type.results.push_back(std::move(cur_token.str));
    nextToken();
  }
  func_type.args = std::move(params);

  BlockStmt *body = blockStmt();

  FuncDecl *func = new FuncDecl();
  func->name = std::move(func_name);
  func->func_type = std::move(func_type);
  func->body = body;
  return func;
}

BlockStmt *Parser::blockStmt() {
  std::vector<Stmt *> stmts;
  takeToken(TokenType::BraceL);
  while (cur_token.type != TokenType::BraceR) {
    stmts.emplace_back(statement());
  }
  takeToken(TokenType::BraceR);
  BlockStmt *block = new BlockStmt();
  block->stmts = std::move(stmts);
  return block;
}

Stmt *Parser::statement() {
  switch (cur_token.type) {
  case TokenType::If:
    return ifStmt();
  case TokenType::Let:
    return letStmt();
  case TokenType::Return:
    return returnStmt();
  default:;
    return nullptr;
  }
}

LetStmt *Parser::letStmt() {
  takeToken(TokenType::Let);
  if (cur_token.type != TokenType::Ident) {
    throw "err";
  }
  std::string ident = std::move(cur_token.str);
  nextToken();
  takeToken(TokenType::Assign);
  Expr *expr_ = expr();

  LetStmt *stmt = new LetStmt();
  stmt->ident = std::move(ident);
  stmt->expr = expr_;
  return stmt;
}

ReturnStmt *Parser::returnStmt() {
  takeToken(TokenType::Return);
  Expr *result = expr();
  ReturnStmt *stmt = new ReturnStmt();
  stmt->results.push_back(result);
  return stmt;
}

// void Parser::statementAssign() {
//   const auto &info = ident_table.find(cur_token.ident);
//   llvm::Value *assignee;
//   if (info.type == pl0llvm::IdType::Var) {
//     assignee = info.val;
//   } else {
//     error("variable is expected but it is not variable");
//   }
//   next();
//   takeToken(TokenType::Assign);
//   builder.CreateStore(expression(), assignee);
//   return;
// }

IfStmt *Parser::ifStmt() {
  takeToken(TokenType::If);

  auto *cond = expr();
  auto *then_block = blockStmt();

  IfStmt *stmt = new IfStmt();
  stmt->cond = cond;
  stmt->then_block = then_block;
  stmt->else_block = nullptr;
  return stmt;
}

// void Parser::statementWhile() {
//   takeToken(TokenType::While);

//   auto *cond_block = llvm::BasicBlock::Create(context, "while.cond",
//   curFunc); auto *body_block = llvm::BasicBlock::Create(context,
//   "while.body"); auto *merge_block = llvm::BasicBlock::Create(context,
//   "while.merge");

//   builder.CreateBr(cond_block);

//   {
//     builder.SetInsertPoint(cond_block);
//     auto *cond = condition();
//     takeToken(TokenType::Do);
//     builder.CreateCondBr(cond, body_block, merge_block);
//   }

//   {
//     curFunc->getBasicBlockList().push_back(body_block);
//     builder.SetInsertPoint(body_block);
//     statement();
//     builder.CreateBr(cond_block);
//   }

//   curFunc->getBasicBlockList().push_back(merge_block);
//   builder.SetInsertPoint(merge_block);
// }

// llvm::CmpInst::Predicate token_to_inst(TokenType type) {
//   switch (type) {
//   case TokenType::Equal:
//     return llvm::CmpInst::Predicate::ICMP_EQ;
//   case TokenType::NotEqual:
//     return llvm::CmpInst::Predicate::ICMP_NE;
//   case TokenType::Less:
//     return llvm::CmpInst::Predicate::ICMP_SLT;
//   case TokenType::LessEqual:
//     return llvm::CmpInst::Predicate::ICMP_SLE;
//   case TokenType::Greater:
//     return llvm::CmpInst::Predicate::ICMP_SGT;
//   case TokenType::GreaterEqual:
//     return llvm::CmpInst::Predicate::ICMP_SGE;
//   default:
//     throw "not support at token to inst";
//   }
// }

// llvm::Value *Parser::condition() {
//   if (cur_token.type == TokenType::Odd) {
//     next();
//     auto *lhs = builder.CreateSRem(expression(), builder.getInt64(2));
//     return builder.CreateICmpEQ(lhs, builder.getInt64(0));
//   } else {
//     auto *lhs = expression();
//     llvm::CmpInst::Predicate inst = token_to_inst(cur_token.type);
//     next();
//     auto *rhs = expression();
//     return builder.CreateICmp(inst, lhs, rhs);
//   }
// }

Expr *Parser::expr() { return factor(); }

Expr *Parser::factor() {
  Expr *ret = parseOperand();

  if (cur_token.type == TokenType::ParenL) {
    return parseCallExpr(ret);
  }
  return ret;
}

Expr *Parser::parseOperand() {
  if (cur_token.type == TokenType::Integer) {
    BasicLit *lit = new BasicLit();
    lit->kind = cur_token.type;
    lit->value = std::move(cur_token.str);
    nextToken();
    return lit;
  } else if (cur_token.type == TokenType::Ident) {
    Ident *ident = new Ident();
    ident->name = std::move(cur_token.str);
    nextToken();
    return ident;
  }
  return nullptr;
}

CallExpr *Parser::parseCallExpr(Expr *callee) {
  std::vector<Expr *> args;
  takeToken(TokenType::ParenL);
  while (true) {
    args.push_back(expr());
    if (cur_token.type == TokenType::Comma) {
      nextToken();
    } else {
      break;
    }
  }
  takeToken(TokenType::ParenR);
  CallExpr *callExpr = new CallExpr();
  callExpr->func = callee;
  callExpr->args = std::move(args);
  return callExpr;
}
