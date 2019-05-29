#include <sstream>
#include <string>

#include "./error.hpp"
#include "./parser.hpp"

using namespace yslang;

Parser::Parser(const std::string &input) : lexer(input) {
  next_token();
  next_token();

  prefix_parse_functions[TokenType::Integer] = &Parser::parse_literal;
  prefix_parse_functions[TokenType::String] = &Parser::parse_literal;
  prefix_parse_functions[TokenType::Ident] = &Parser::parse_identifier;

  infix_parse_functions[TokenType::Plus] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Minus] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Mul] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Div] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Equal] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::NotEqual] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Greater] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::GreaterEqual] =
      &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Assign] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::Less] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::LessEqual] = &Parser::parse_infix_expression;
  infix_parse_functions[TokenType::ParenL] = &Parser::parse_call_expression;
  infix_parse_functions[TokenType::Dot] = &Parser::parse_ref_expression;
  infix_parse_functions[TokenType::BracketL] = &Parser::parse_index_expression;

  precedences[TokenType::Plus] = Precedence::SUM;
  precedences[TokenType::Minus] = Precedence::SUM;
  precedences[TokenType::Mul] = Precedence::PRODUCT;
  precedences[TokenType::Div] = Precedence::PRODUCT;
  precedences[TokenType::Equal] = Precedence::EQUALS;
  precedences[TokenType::NotEqual] = Precedence::EQUALS;
  precedences[TokenType::Greater] = Precedence::EQUALS;
  precedences[TokenType::GreaterEqual] = Precedence::EQUALS;
  precedences[TokenType::Less] = Precedence::EQUALS;
  precedences[TokenType::LessEqual] = Precedence::EQUALS;
  precedences[TokenType::ParenL] = Precedence::CALL;
  precedences[TokenType::Dot] = Precedence::CALL;
  precedences[TokenType::Assign] = Precedence::ASSIGN;
  precedences[TokenType::BracketL] = Precedence::INDEX;
}

Program Parser::parse() {
  Program program;

  while (cur_token.type != TokenType::TEOF) {
    Decl *decl = parse_decl();
    program.decls.push_back(decl);
  }

  return program;
}

// -------------------- //
// Decl
// -------------------- //

Decl *Parser::parse_decl() {
  std::stringstream ss;
  switch (cur_token.type) {
  case TokenType::Func:
    return parse_func_decl();
  case TokenType::Import:
    return parse_import_decl();
  case TokenType::Type:
    return parse_type_decl();
  default:
    ss << "unexpected token at parse(): " << cur_token;
    error(ss.str());
  }
  return nullptr;
}

FuncDecl *Parser::parse_func_decl() {
  expect(TokenType::Func);

  std::string func_name = "";
  if (cur_token_is(TokenType::Ident)) {
    func_name = std::move(cur_token.str);
  }

  next_token();

  FunctionType *func_type = parse_function_type();

  BlockStmt *body = parse_block_stmt();

  FuncDecl *func = new FuncDecl();
  func->name = std::move(func_name);
  func->func_type = func_type;
  func->body = body;
  return func;
}

ImportDecl *Parser::parse_import_decl() {
  ImportDecl *decl = new ImportDecl();

  expect(TokenType::Import);
  decl->package = parse_identifier();

  return decl;
}

TypeDecl *Parser::parse_type_decl() {
  expect(TokenType::Type);

  TypeDecl *type_decl = new TypeDecl();
  type_decl->name = parse_identifier();
  type_decl->type = parse_type();

  return type_decl;
}

// -------------------- //
// Type
// -------------------- //

Type *Parser::parse_type() {
  std::stringstream ss;
  switch (cur_token.type) {
  case TokenType::Ident:
    return parse_ident_type();
  case TokenType::Struct:
    return parse_struct_type();
  case TokenType::ParenL:
    return parse_function_type();
  case TokenType::BracketL:
    return parse_array_type();
  default:
    ss << "unexpected type token at parse(): " << cur_token;
    error(ss.str());
    return nullptr;
  }
}

IdentType *Parser::parse_ident_type() {
  IdentType *ident_type = new IdentType();

  ident_type->name = parse_identifier();

  return ident_type;
}

StructType *Parser::parse_struct_type() {
  expect(TokenType::Struct);

  StructType *struct_type = new StructType();
  struct_type->fields = parse_fields();

  return struct_type;
}

FunctionType *Parser::parse_function_type() {
  FunctionType *func_type = new FunctionType();
  func_type->fields = parse_params();
  func_type->result = parse_type();

  return func_type;
}

ArrayType *Parser::parse_array_type() {
  ArrayType *array_type = new ArrayType();

  expect(TokenType::BracketL);
  array_type->length = parse_literal();
  expect(TokenType::BracketR);

  array_type->element = parse_type();

  return array_type;
}

std::vector<Field> Parser::parse_params() {
  expect(TokenType::ParenL);

  std::vector<Field> fields;
  if (!cur_token_is(TokenType::ParenR)) {
    while (true) {
      fields.push_back(parse_param());
      if (!cur_token_is(TokenType::Comma)) {
        break;
      }
      next_token();
    }
  }

  expect(TokenType::ParenR);

  return fields;
}

std::vector<Field> Parser::parse_fields() {
  expect(TokenType::BraceL);

  std::vector<Field> fields;
  while (!cur_token_is(TokenType::BraceR)) {
    fields.push_back(parse_param());
    expect(TokenType::Semicolon);
  }

  expect(TokenType::BraceR);

  return fields;
}

Field Parser::parse_param() {
  Field field;

  field.name = parse_identifier();
  field.type = parse_type();

  return field;
}

// -------------------- //
// Stmt
// -------------------- //

BlockStmt *Parser::parse_block_stmt() {
  expect(TokenType::BraceL);

  std::vector<Stmt *> stmts;
  while (!cur_token_is(TokenType::BraceR) && !cur_token_is(TokenType::TEOF)) {
    stmts.push_back(parse_statement());
  }

  expect(TokenType::BraceR);

  BlockStmt *block = new BlockStmt();
  block->stmts = std::move(stmts);
  return block;
}

Stmt *Parser::parse_statement() {
  switch (cur_token.type) {
  case TokenType::If:
    return parse_if_statement();
  case TokenType::Let:
    return parse_let_stmt();
  case TokenType::Return:
    return parse_return_stmt();
  default:
    return parse_expression_stmt();
  }
}

LetStmt *Parser::parse_let_stmt() {
  expect(TokenType::Let);

  LetStmt *stmt = new LetStmt();
  stmt->ident = parse_identifier();
  stmt->type = parse_type();
  stmt->expr = nullptr;

  if (cur_token_is(TokenType::Assign)) {
    stmt->expr = parse_expression(LOWEST);
  }

  expect(TokenType::Semicolon);

  return stmt;
}

ReturnStmt *Parser::parse_return_stmt() {
  expect(TokenType::Return);

  Expr *result = parse_expression(LOWEST);

  ReturnStmt *stmt = new ReturnStmt();
  stmt->results.push_back(result);

  expect(TokenType::Semicolon);

  return stmt;
}

IfStmt *Parser::parse_if_statement() {
  expect(TokenType::If);

  auto *cond = parse_expression(LOWEST);
  auto *then_block = parse_block_stmt();

  Stmt *else_block = nullptr;
  if (cur_token_is(TokenType::Else)) {
    expect(TokenType::Else);

    if (cur_token_is(TokenType::If)) {
      else_block = parse_if_statement();
    } else {
      else_block = parse_block_stmt();
    }
  }

  IfStmt *stmt = new IfStmt();
  stmt->cond = cond;
  stmt->then_block = then_block;
  stmt->else_block = else_block;
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

ExprStmt *Parser::parse_expression_stmt() {
  ExprStmt *stmt = new ExprStmt();

  stmt->expr = parse_expression(LOWEST);

  expect(TokenType::Semicolon);
  return stmt;
}

Expr *Parser::parse_expression(Precedence precedence) {
  auto prefix = prefix_parse_functions[cur_token.type];

  if (prefix == nullptr) {
    return nullptr;
  }

  Expr *left_expr = prefix(this);

  while (!cur_token_is(TokenType::Semicolon) && precedence < cur_precedence()) {
    auto infix = infix_parse_functions[cur_token.type];
    if (infix == nullptr) {
      return left_expr;
    }

    left_expr = infix(this, left_expr);
  }

  return left_expr;
}

BasicLit *Parser::parse_literal() {
  BasicLit *lit = new BasicLit();
  lit->kind = cur_token.type;
  lit->value = std::move(cur_token.str);

  next_token();

  return lit;
}

Ident *Parser::parse_identifier() {
  Ident *ident = new Ident();
  ident->name = "_";

  if (cur_token_is(TokenType::Ident)) {
    ident->name = std::move(cur_token.str);
    next_token();
  } else {
    expect(TokenType::Ident); // for error handling
  }

  return ident;
}

Expr *Parser::parse_infix_expression(Expr *left) {
  BinaryExpr *expression = new BinaryExpr();

  expression->lhs = left;
  expression->op = cur_token.type;

  Precedence precedences = cur_precedence();
  next_token();
  expression->rhs = parse_expression(precedences);

  return expression;
}

Expr *Parser::parse_call_expression(Expr *left) {
  expect(TokenType::ParenL);

  std::vector<Expr *> args;
  if (!cur_token_is(TokenType::ParenR)) {
    args = parse_expression_list();
  }

  expect(TokenType::ParenR);

  CallExpr *expr = new CallExpr;
  expr->func = left;
  expr->args = std::move(args);
  return expr;
}

Expr *Parser::parse_ref_expression(Expr *left) {
  RefExpr *expr = new RefExpr();
  expr->receiver = left;

  expect(TokenType::Dot);
  expr->ref = parse_identifier();

  return expr;
}

Expr *Parser::parse_index_expression(Expr *left) {
  IndexExpr *expr = new IndexExpr();
  expr->receiver = left;

  expect(TokenType::BracketL);
  expr->index = parse_expression(LOWEST);
  expect(TokenType::BracketR);

  return expr;
}

std::vector<Expr *> Parser::parse_expression_list() {
  std::vector<Expr *> list;

  list.emplace_back(parse_expression(LOWEST));
  while (cur_token_is(TokenType::Comma)) {
    next_token();
    list.emplace_back(parse_expression(LOWEST));
  }

  return list;
}
