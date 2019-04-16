#include <sstream>
#include <string>

#include "./error.hpp"
#include "./parser.hpp"

using namespace yslang;

Parser::Parser(const std::string &input) : lexer(input) {
  next_token();
  next_token();

  prefix_parse_functions[TokenType::Integer] = &Parser::parseIntegerLiteral;
}

Program Parser::parse() {
  Program program;

  while (cur_token.type != TokenType::TEOF) {
    Decl *decl = parse_decl();
    program.decls.push_back(decl);
    next_token();
  }

  return program;
}

Decl *Parser::parse_decl() {
  std::stringstream ss;
  switch (cur_token.type) {
  case TokenType::Func:
    return parse_func_decl();
  // case TokenType::Const:
  //   return constDecl();
  default:
    ss << "unexpected token at parse(): " << cur_token;
    error(ss.str());
  }
  return nullptr;
}

// ConstDecl *Parser::constDecl() {
//   takeToken(TokenType::Const);
//   if (cur_token.type != TokenType::Ident) {
//     error("expected Ident at constDecl()");
//   }
//   std::string name = std::move(cur_token.str);
//   next_token();

//   takeToken(TokenType::Assign);
//   Expr *body = expr();

//   ConstDecl *decl = new ConstDecl();
//   decl->name = std::move(name);
//   decl->expr = body;
//   return decl;
// }

FuncDecl *Parser::parse_func_decl() {
  if (!expect_peek(TokenType::Ident)) {
    error("no Ident at funcDecl()");
  }

  std::string func_name = std::move(cur_token.str);
  next_token();

  FuncType func_type = parse_func_type();

  BlockStmt *body = blockStmt();

  FuncDecl *func = new FuncDecl();
  func->name = std::move(func_name);
  func->func_type = std::move(func_type);
  func->body = body;
  return func;
}

FuncType Parser::parse_func_type() {
  FuncType func_type;

  func_type.fields = parse_params();

  Ident ret;
  ret.name = cur_token.str;
  func_type.results.push_back(std::move(ret));

  next_token();

  return func_type;
}

std::vector<Field> Parser::parse_params() {
  std::vector<Field> fields;

  if (peek_token_is(TokenType::ParenR)) {
    next_token();
    return fields;
  }

  next_token();

  fields.push_back(parse_param());

  while (peek_token_is(TokenType::Comma)) {
    next_token();
    next_token();
    fields.push_back(parse_param());
  }

  if (expect_peek(TokenType::ParenR)) {
    return {};
  }

  return fields;
}

Field Parser::parse_param() {
  Field field;
  field.name.name = cur_token.str;
  field.type.name = peek_token.str;
  next_token();
  return field;
}

BlockStmt *Parser::blockStmt() {
  std::vector<Stmt *> stmts;
  if (!expect_peek(TokenType::BraceL)) {
    return nullptr;
  }

  next_token();

  while (!cur_token_is(TokenType::BraceR) && !cur_token_is(TokenType::TEOF)) {
    Stmt *stmt = statement();
    if (stmt != nullptr) {
      stmts.push_back(stmt);
    }
    next_token();
  }

  BlockStmt *block = new BlockStmt();
  block->stmts = std::move(stmts);
  return block;
}

Stmt *Parser::statement() {
  switch (cur_token.type) {
  // case TokenType::If:
  //   return ifStmt();
  // case TokenType::Let:
  //   return letStmt();
  case TokenType::Return:
    return returnStmt();
  default:;
    return nullptr;
  }
}

// LetStmt *Parser::letStmt() {
//   takeToken(TokenType::Let);
//   if (cur_token.type != TokenType::Ident) {
//     throw "err";
//   }
//   std::string ident = std::move(cur_token.str);
//   next_token();
//   takeToken(TokenType::Assign);
//   Expr *expr_ = expr();

//   LetStmt *stmt = new LetStmt();
//   stmt->ident = std::move(ident);
//   stmt->expr = expr_;
//   return stmt;
// }

ReturnStmt *Parser::returnStmt() {
  next_token();

  Expr *result = expr(0);

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

// IfStmt *Parser::ifStmt() {
//   takeToken(TokenType::If);

//   auto *cond = expr();
//   auto *then_block = blockStmt();

//   IfStmt *stmt = new IfStmt();
//   stmt->cond = cond;
//   stmt->then_block = then_block;
//   stmt->else_block = nullptr;
//   return stmt;
// }

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

Expr *Parser::expr(int precedence) {
  auto prefix = prefix_parse_functions[cur_token.type];
  // if prefix == nil {
  // 	p.noPrefixParseFnError(p.curToken.Type)
  // 	return nil
  // }
  Expr *leftExp = prefix(this);

  while (peek_token_is(TokenType::Semicolon) &&
         precedence < peek_precedence()) {
    auto infix = infix_parse_functions[peek_token.type];
    // if infix == nil {
    // 	return leftExp
    // }

    next_token();

    leftExp = infix(this, leftExp);
  }

  return leftExp;
}

// Expr *Parser::binaryExpr() {
//   Expr *lhs = factor();
//   while (cur_token.isOP()) {
//     auto op = cur_token.type;
//     next_token();
//     Expr *rhs = binaryExpr();
//     BinaryExpr *new_lhs = new BinaryExpr();
//     new_lhs->lhs = lhs;
//     new_lhs->op = op;
//     new_lhs->rhs = rhs;
//     lhs = (Expr *)new_lhs;
//   }
//   return lhs;
// }

// Expr *Parser::factor() {
//   Expr *ret = parseOperand();

//   if (cur_token.type == TokenType::ParenL) {
//     return parseCallExpr(ret);
//   }
//   return ret;
// }

// Expr *Parser::parseOperand() {
//   if (cur_token.type == TokenType::Integer) {
//     BasicLit *lit = new BasicLit();
//     lit->kind = cur_token.type;
//     lit->value = std::move(cur_token.str);
//     next_token();
//     return lit;
//   } else if (cur_token.type == TokenType::Ident) {
//     Ident *ident = new Ident();
//     ident->name = std::move(cur_token.str);
//     next_token();
//     return ident;
//   }
//   return nullptr;
// }

Expr *Parser::parseIntegerLiteral() {
  BasicLit *lit = new BasicLit();
  lit->kind = cur_token.type;
  lit->value = std::move(cur_token.str);
  next_token();
  return lit;
}

// CallExpr *Parser::parseCallExpr(Expr *callee) {
//   std::vector<Expr *> args;
//   takeToken(TokenType::ParenL);
//   while (cur_token.type != TokenType::ParenR) {
//     args.push_back(expr());
//     if (cur_token.type == TokenType::Comma) {
//       next_token();
//     } else {
//       break;
//     }
//   }
//   takeToken(TokenType::ParenR);
//   CallExpr *callExpr = new CallExpr();
//   callExpr->func = callee;
//   callExpr->args = std::move(args);
//   return callExpr;
// }
