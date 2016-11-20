/*
 * MIT License
 *
 * Copyright (c) 2016 morrisfeist
 * Copyright (c) 2016 tpriesner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "parser.hpp"

#include "ast.hpp"
#include "dotvisitor.hpp"
#include "lexer.hpp"
#include "pprinter.hpp"

#include <deque>

using TT = Token::Type;

void Parser::parseFileOnly() { parseProgram(); }
void Parser::parseAndPrintAst() {
  auto program = parseProgram();
  PrettyPrinterVisitor v(std::cout, "\t");
  program->accept(&v);
}
void Parser::parseAndDotAst() {
  auto program = parseProgram();
  DotVisitor dotVisitor{std::cout};
  program->accept(&dotVisitor);
}

ast::ProgramPtr Parser::parseProgram() {
  std::vector<ast::ClassPtr> classes;
  auto startPos = curTok.startPos();
  while (true) {
    switch (curTok.type) {
    case TT::Class:
      classes.emplace_back(parseClassDeclaration());
      break;
    case TT::Eof: {
      auto endPos = curTok.startPos();
      return ast::make_Ptr<ast::Program>({startPos, endPos},
                                         std::move(classes));
    }
    default:
      errorExpectedAnyOf({TT::Class, TT::Eof});
      break;
    }
  }
}

ast::ClassPtr Parser::parseClassDeclaration() {
  std::string name;
  std::vector<ast::FieldPtr> fields;
  std::vector<ast::RegularMethodPtr> methods;
  std::vector<ast::MainMethodPtr> mainMethods;

  auto startPos = curTok.startPos();
  expectAndNext(TT::Class);
  name = expectGetIdentAndNext(TT::Identifier);
  expectAndNext(TT::LBrace);
  // parse class members:
  while (true) {
    switch (curTok.type) {
    case TT::RBrace: {
      auto endPos = curTok.endPos();
      readNextToken();
      return ast::make_Ptr<ast::Class>({startPos, endPos}, std::move(name),
                                       std::move(fields), std::move(methods),
                                       std::move(mainMethods));
    }
    case TT::Public:
      parseClassMember(fields, methods, mainMethods);
      break;
    default:
      errorExpectedAnyOf({TT::RBrace, TT::Public});
      break;
    }
  }
}

void Parser::parseClassMember(std::vector<ast::FieldPtr> &fields,
                              std::vector<ast::RegularMethodPtr> &methods,
                              std::vector<ast::MainMethodPtr> &mainMethods) {
  switch (lookAhead(1).type) {
  case TT::Static:
    mainMethods.emplace_back(parseMainMethod());
    return;
  case TT::Boolean:
  case TT::Identifier:
  case TT::Int:
  case TT::Void:
    parseFieldOrMethod(fields, methods);
    return;
  default:
    errorExpectedAnyOf(
        {TT::Static, TT::Boolean, TT::Identifier, TT::Int, TT::Void});
    break;
  }
}

ast::MainMethodPtr Parser::parseMainMethod() {
  auto startPos = curTok.startPos();
  expectAndNext(TT::Public);
  expectAndNext(TT::Static);
  expectAndNext(TT::Void);
  // name must be main (check in semantic analysis):
  auto methodName = expectGetIdentAndNext(TT::Identifier);
  expectAndNext(TT::LParen);
  expect(TT::Identifier);
  if (curTok.str != "String") {
    error("Unexpected '" + curTok.str + "', expected 'String'");
  }
  readNextToken();
  expectAndNext(TT::LBracket);
  expectAndNext(TT::RBracket);
  // name doesn't matter here, but keep for pretty print:
  auto paramName = expectGetIdentAndNext(TT::Identifier);
  expectAndNext(TT::RParen);
  auto block = parseBlock();
  return ast::make_Ptr<ast::MainMethod>(
      {startPos, curTok.endPos()}, std::move(methodName),
      strTbl.findOrInsert(paramName), std::move(block));
}

void Parser::parseFieldOrMethod(std::vector<ast::FieldPtr> &fields,
                                std::vector<ast::RegularMethodPtr> &methods) {
  auto startPos = curTok.startPos();
  expectAndNext(TT::Public);
  auto type = parseType();
  auto name = expectGetIdentAndNext(TT::Identifier);
  switch (curTok.type) {
  case TT::Semicolon: // field
    fields.emplace_back(ast::make_Ptr<ast::Field>({startPos, curTok.endPos()},
                                                  std::move(type),
                                                  strTbl.findOrInsert(name)));
    readNextToken();
    return;
  case TT::LParen: { // method
    readNextToken();
    auto params = parseParameterList();
    expectAndNext(TT::RParen);
    auto block = parseBlock();
    methods.emplace_back(ast::make_Ptr<ast::RegularMethod>(
        {startPos, block->getLoc().endToken}, std::move(type), std::move(name),
        std::move(params), std::move(block)));
    return;
  }
  default:
    errorExpectedAnyOf({TT::Semicolon, TT::LParen});
    break;
  }
}

ast::ParameterList Parser::parseParameterList() {
  ast::ParameterList params;
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Identifier:
  case TT::Int:
  case TT::Void:
    params.emplace_back(parseParameter());
    while (true) {
      switch (curTok.type) {
      case TT::Comma:
        readNextToken();
        params.emplace_back(parseParameter());
        break;
      default:
        return params;
      }
    }
    break;
  default:
    return {}; // empty vector
  }
}

ast::ParameterPtr Parser::parseParameter() {
  auto startPos = curTok.startPos();
  auto type = parseType();
  auto endPos = curTok.endPos();
  auto ident = expectGetIdentAndNext(TT::Identifier);
  return ast::make_Ptr<ast::Parameter>({startPos, endPos}, std::move(type),
                                       strTbl.findOrInsert(ident));
}

ast::TypePtr Parser::parseType() {
  auto startPos = curTok.startPos();
  auto endPos = curTok.endPos();
  auto basicType = parseBasicType();
  int numDimensions = 0;
  while (true) {
    switch (curTok.type) {
    case TT::LBracket:
      readNextToken();
      endPos = curTok.endPos();
      expectAndNext(TT::RBracket);
      numDimensions++;
      break;
    default:
      if (numDimensions == 0) {
        return std::move(basicType);
      } else {
        return ast::make_Ptr<ast::ArrayType>(
            {startPos, endPos}, std::move(basicType), numDimensions);
      }
    }
  }
}

ast::BasicTypePtr Parser::parseBasicType() {
  auto loc = curTok.singleTokenSrcLoc();
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Int:
  case TT::Void: {
    auto type = curTok.type;
    readNextToken();
    auto typeType = ast::PrimitiveType::getTypeForToken(type);
    return ast::make_Ptr<ast::PrimitiveType>(loc, typeType);
  }
  case TT::Identifier: {
    auto ident = std::move(curTok.str);
    readNextToken();
    return ast::make_Ptr<ast::ClassType>(loc, std::move(ident));
  }
  default:
    errorExpectedAnyOf({TT::Boolean, TT::Identifier, TT::Int, TT::Void});
    break;
  }
}

ast::BlockPtr Parser::parseBlock() {
  auto startPos = curTok.startPos();
  ast::BlockStmtList statements;

  expectAndNext(TT::LBrace);
  while (true) {
    switch (curTok.type) {
    case TT::RBrace: {
      auto endPos = curTok.endPos();
      readNextToken();
      bool containsNothingExceptOneSingleLonelyEmtpyExpression = true;

      for (ast::BlockStmtList::size_type i = 0; i < statements.size(); i++) {
        if (statements[i] == nullptr) {
          // do nothing
        } else if (ast::Block *b =
                       dynamic_cast<ast::Block *>(statements[i].get())) {
          if (!b->getContainsNothingExceptOneSingleLonelyEmptyExpression()) {
            containsNothingExceptOneSingleLonelyEmtpyExpression = false;
          }
        } else {
          containsNothingExceptOneSingleLonelyEmtpyExpression = false;
        }
      }

      return ast::make_Ptr<ast::Block>(
          {startPos, endPos}, std::move(statements),
          containsNothingExceptOneSingleLonelyEmtpyExpression);
    }
    case TT::Bang:
    case TT::Boolean:
    case TT::False:
    case TT::Identifier:
    case TT::If:
    case TT::Int:
    case TT::IntLiteral:
    case TT::New:
    case TT::Null:
    case TT::True:
    case TT::LBrace:
    case TT::LParen:
    case TT::Minus:
    case TT::Return:
    case TT::Semicolon:
    case TT::This:
    case TT::Void:
    case TT::While: {
      if (auto stmt = parseBlockStatement()) {
        statements.emplace_back(std::move(stmt));
      }
      break;
    }
    default:
      errorExpectedAnyOf({TT::RBrace, TT::Bang, TT::Boolean, TT::False,
                          TT::Identifier, TT::If, TT::Int, TT::IntLiteral,
                          TT::New, TT::Null, TT::True, TT::LBrace, TT::LParen,
                          TT::Minus, TT::Return, TT::Semicolon, TT::This,
                          TT::Void, TT::While});
      break;
    }
  }
}

ast::BlockStmtPtr Parser::parseBlockStatement() {
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Int:
  case TT::Void:
    return parseLocalVarDeclStmt();
  case TT::Bang:
  case TT::False:
  case TT::If:
  case TT::IntLiteral:
  case TT::New:
  case TT::Null:
  case TT::True:
  case TT::LBrace:
  case TT::LParen:
  case TT::Minus:
  case TT::Return:
  case TT::Semicolon:
  case TT::This:
  case TT::While:
    return parseStmt();
  case TT::Identifier: {
    Token &nextTok = lookAhead(1);
    Token &afterNextTok = lookAhead(2);
    if (nextTok.type == TT::Identifier ||
        (nextTok.type == TT::LBracket && afterNextTok.type == TT::RBracket)) {
      return parseLocalVarDeclStmt();
    } else {
      return parseStmt();
    }
    break;
  }
  default:
    return nullptr; // return null if block is empty
  }
}

ast::BlockStmtPtr Parser::parseLocalVarDeclStmt() {
  auto startPos = curTok.startPos();
  auto type = parseType();
  auto ident = expectGetIdentAndNext(TT::Identifier);
  switch (curTok.type) {
  case TT::Eq: {
    auto endPos = curTok.endPos();
    readNextToken();
    auto initializer = parseExpr();
    if (initializer) {
      endPos = initializer->getLoc().endToken;
    }
    expectAndNext(TT::Semicolon);
    return ast::make_Ptr<ast::VariableDeclaration>(
        {startPos, endPos}, std::move(type), strTbl.findOrInsert(ident),
        std::move(initializer));
  }
  case TT::Semicolon: {
    auto endPos = curTok.endPos();
    readNextToken();
    return ast::make_Ptr<ast::VariableDeclaration>(
        {startPos, endPos}, std::move(type), strTbl.findOrInsert(ident),
        nullptr);
  }
  default:
    errorExpectedAnyOf({TT::Eq, TT::Semicolon});
  }
}

// might return nullptr if statement is empty
ast::StmtPtr Parser::parseStmt() {
  switch (curTok.type) {
  case TT::LBrace:
    return parseBlock();
  case TT::Semicolon:
    readNextToken(); // empty statement -> ignore
    return nullptr;  // return null for empty statements
  case TT::If:
    return parseIfStmt();
  case TT::Return:
    return parseReturnStmt();
  case TT::While:
    return parseWhileStmt();
  case TT::Bang:
  case TT::False:
  case TT::IntLiteral:
  case TT::New:
  case TT::Null:
  case TT::True:
  case TT::LParen:
  case TT::Minus:
  case TT::This:
  case TT::Identifier:
    return parseExprStmt();
  default:
    errorExpectedAnyOf({TT::LBrace, TT::Semicolon, TT::If, TT::Return,
                        TT::While, TT::Bang, TT::False, TT::IntLiteral, TT::New,
                        TT::Null, TT::True, TT::LParen, TT::Minus, TT::This,
                        TT::Identifier});
  }
}

ast::StmtPtr Parser::parseExprStmt() {
  auto startPos = curTok.startPos();
  auto expr = parseExpr();
  auto endPos = curTok.endPos();
  expectAndNext(TT::Semicolon);
  return ast::make_Ptr<ast::ExpressionStatement>({startPos, endPos},
                                                 std::move(expr));
}

ast::StmtPtr Parser::parseIfStmt() {
  auto startPos = curTok.startPos();
  ast::StmtPtr elseStmt = nullptr;

  expectAndNext(TT::If); // necessary?
  expectAndNext(TT::LParen);
  auto condition = parseExpr();
  auto endPos = curTok.endPos();
  expectAndNext(TT::RParen);
  auto thenStmt = parseStmt();
  if (curTok.type == TT::Else) {
    readNextToken();
    elseStmt = parseStmt();
  }
  return ast::make_Ptr<ast::IfStatement>(
      {startPos, endPos}, std::move(condition), std::move(thenStmt),
      elseStmt ? std::move(elseStmt) : nullptr);
}

ast::StmtPtr Parser::parseReturnStmt() {
  auto startPos = curTok.startPos();
  expectAndNext(TT::Return);
  switch (curTok.type) {
  case TT::Semicolon: {
    auto endPos = curTok.startPos();
    readNextToken();
    return ast::make_Ptr<ast::ReturnStatement>({startPos, endPos}, nullptr);
  }
  default: {
    auto expr = parseExpr();
    auto endPos = curTok.startPos();
    expectAndNext(TT::Semicolon);
    return ast::make_Ptr<ast::ReturnStatement>({startPos, endPos},
                                               std::move(expr));
  }
  }
}

ast::StmtPtr Parser::parseWhileStmt() {
  auto startPos = curTok.startPos();
  expectAndNext(TT::While); // necessary?
  expectAndNext(TT::LParen);
  auto condition = parseExpr();
  expectAndNext(TT::RParen);
  auto stmt = parseStmt();
  auto endPos = curTok.endPos();

  return ast::make_Ptr<ast::WhileStatement>(
      {startPos, endPos}, std::move(condition), std::move(stmt));
}

ast::ExprPtr Parser::parseExpr() { return precedenceParse(0); }

static int getOpPrec(Token::Type tt) {
  switch (tt) {
  case TT::Eq:
    return 1;
  case TT::VBarVBar:
    return 2;
  case TT::AmpAmp:
    return 3;
  case TT::EqEq:
  case TT::BangEq:
    return 4;
  case TT::Lt:
  case TT::LtEq:
  case TT::Gt:
  case TT::GtEq:
    return 5;
  case TT::Plus:
  case TT::Minus:
    return 6;
  case TT::Star:
  case TT::Slash:
  case TT::Percent:
    return 7;
  default:
    return -1;
  }
}

ast::ExprPtr Parser::precedenceParse(int minPrec) {
  // check precondition!
  auto startPos = curTok.startPos(); // stays the same
  auto result = parseUnary();
  int opPrec;
  while ((opPrec = getOpPrec(curTok.type)) >= minPrec) {
    auto opTok = std::move(curTok);
    readNextToken();
    if (opTok.type != TT::Eq) { // only right assoc case
      opPrec += 1;
    }
    auto rhs = precedenceParse(opPrec);
    auto endPos = rhs->getLoc().endToken;
    auto operation = ast::BinaryExpression::getOpForToken(opTok.type);
    result = ast::make_EPtr<ast::BinaryExpression>(
        {startPos, endPos}, std::move(result), std::move(rhs), operation);
  }
  return result;
}

ast::ExprPtr Parser::parseUnary() {
  std::deque<Token> unaries;
  while (true) {
    switch (curTok.type) {
    case TT::Bang:
      unaries.emplace_back(std::move(curTok));
      readNextToken();
      continue; // parseUnary();
    case TT::Minus:
      unaries.emplace_back(std::move(curTok));
      readNextToken();
      continue; // parseUnary();
      break;
    default:
      if (curTok.type == TT::IntLiteral && !unaries.empty() &&
          unaries.back().type == TT::Minus) {
        curTok.str = "-" + curTok.str;
        unaries.pop_back();
      }
      auto expression = parsePostfixExpr();
      auto endPos = expression->getLoc().endToken;
      // result = ...
      // consume all unary prefixes in reverse order

      for (std::deque<Token>::reverse_iterator i = unaries.rbegin(),
                                               end = unaries.rend();
           i != end; ++i) {

        Token t = std::move(*i);
        auto startPos = t.startPos();
        auto op = ast::UnaryExpression::getOpForToken(t.type);
        expression = ast::make_EPtr<ast::UnaryExpression>(
            {startPos, endPos}, std::move(expression), op);
      }
      return expression;
    }
  }
}

ast::ExprPtr Parser::parsePostfixExpr() {
  auto lhs = parsePrimary();
  while (true) {
    switch (curTok.type) {
    case TT::Dot:
      lhs = parseMemberAccess(std::move(lhs));
      break;
    case TT::LBracket:
      lhs = parseArrayAccess(std::move(lhs));
      break;
    default:
      return lhs;
    }
  }
}

ast::ExprPtr Parser::parsePrimary() {
  switch (curTok.type) {
  case TT::LParen: {
    readNextToken();
    auto expr = parseExpr();
    expectAndNext(TT::RParen);
    return expr;
  }
  case TT::False: {
    auto loc = curTok.singleTokenSrcLoc();
    readNextToken();
    return ast::make_EPtr<ast::BoolLiteral>(loc, false);
  }
  case TT::True: {
    auto loc = curTok.singleTokenSrcLoc();
    readNextToken();
    return ast::make_EPtr<ast::BoolLiteral>(loc, true);
  }
  case TT::Null: {
    auto loc = curTok.singleTokenSrcLoc();
    readNextToken();
    return ast::make_EPtr<ast::NullLiteral>(loc);
  }
  case TT::This: {
    auto loc = curTok.singleTokenSrcLoc();
    readNextToken();
    return ast::make_EPtr<ast::ThisLiteral>(loc);
  }
  case TT::IntLiteral: {
    auto loc = curTok.singleTokenSrcLoc();
    int32_t value;
    try {
      value = std::stoi(curTok.str);
    } catch (std::out_of_range &o) {
      error("Integer literal '" + curTok.str + "' out of range");
    } catch (std::invalid_argument &i) {
      // should never happen!
      throw std::runtime_error("Invalid integer literal");
    }
    readNextToken();
    return ast::make_EPtr<ast::IntLiteral>(loc, value);
  }
  case TT::Identifier: {
    auto startPos = curTok.startPos();
    auto fieldEndPos = curTok.endPos();
    auto ident = std::move(curTok.str);
    readNextToken();
    if (curTok.type == TT::LParen) {
      // return "this.methodinvocation(args)
      readNextToken();
      auto args = parseArguments();
      auto endPos = curTok.endPos();
      expectAndNext(TT::RParen);
      SourceLocation loc{startPos, endPos};
      return ast::make_EPtr<ast::MethodInvocation>(
          loc, ast::make_EPtr<ast::ThisLiteral>(loc), std::move(ident),
          std::move(args));
    } else {
      SourceLocation loc{startPos, fieldEndPos};
      return ast::make_EPtr<ast::VarRef>(loc, strTbl.findOrInsert(ident));
    }
  }
  case TT::New:
    return parseNewExpr();
  default:
    errorExpectedAnyOf({TT::LParen, TT::False, TT::True, TT::Null, TT::This,
                        TT::IntLiteral, TT::Identifier, TT::New});
  }
}

ast::ExprPtr Parser::parseMemberAccess(ast::ExprPtr lhs) {
  auto startPos = curTok.startPos();
  expectAndNext(TT::Dot);
  auto endPos = curTok.endPos();
  auto ident = expectGetIdentAndNext(TT::Identifier);
  if (curTok.type == TT::LParen) {
    readNextToken();
    auto args = parseArguments();
    endPos = curTok.endPos();
    expectAndNext(TT::RParen);
    return ast::make_EPtr<ast::MethodInvocation>(
        {startPos, endPos}, std::move(lhs), std::move(ident), std::move(args));
  } else {
    return ast::make_EPtr<ast::FieldAccess>({startPos, endPos}, std::move(lhs),
                                            std::move(ident));
  }
}

ast::ExprPtr Parser::parseArrayAccess(ast::ExprPtr lhs) {
  auto startPos = curTok.startPos();
  expectAndNext(TT::LBracket);
  auto index = parseExpr();
  expectAndNext(TT::RBracket);
  auto endPos = curTok.endPos();
  return ast::make_EPtr<ast::ArrayAccess>({startPos, endPos}, std::move(lhs),
                                          std::move(index));
}

ast::ExprList Parser::parseArguments() {
  ast::ExprList arguments;
  switch (curTok.type) {
  case TT::Bang:
  case TT::False:
  case TT::Identifier:
  case TT::IntLiteral:
  case TT::New:
  case TT::Null:
  case TT::True:
  case TT::LParen:
  case TT::Minus:
  case TT::This:
    arguments.emplace_back(parseExpr());
    while (curTok.type == TT::Comma) {
      readNextToken();
      arguments.emplace_back(parseExpr());
    }
    return arguments;
  case TT::RParen:
    return arguments; // empty vector
  default:
    errorExpectedAnyOf({TT::Bang, TT::False, TT::Identifier, TT::IntLiteral,
                        TT::New, TT::Null, TT::True, TT::LParen, TT::Minus,
                        TT::This, TT::RParen});
  }
}

ast::ExprPtr Parser::parseNewExpr() {
  auto startPos = curTok.startPos();
  expectAndNext(TT::New);
  // check next token instead of current
  switch (lookAhead(1).type) {
  case TT::LParen: {
    auto ident = expectGetIdentAndNext(TT::Identifier);
    // curTok is always LParen
    readNextToken();
    auto endPos = curTok.startPos();
    expectAndNext(TT::RParen);
    return ast::make_EPtr<ast::NewObjectExpression>({startPos, endPos}, ident);
  }
  case TT::LBracket: {
    auto elementType = parseBasicType();
    // curTok is always LBracket
    readNextToken(); // eat '['
    auto size = parseExpr();
    auto endPos = curTok.endPos();
    expectAndNext(TT::RBracket);
    int dimension = 1;
    while (curTok.type == TT::LBracket && lookAhead(1).type == TT::RBracket) {
      dimension += 1;
      readNextToken(); // eat '['
      endPos = curTok.endPos();
      readNextToken(); // eat ']'
    }
    SourceLocation loc{startPos, endPos};
    auto arrayType =
        ast::make_Ptr<ast::ArrayType>(loc, std::move(elementType), dimension);
    return ast::make_EPtr<ast::NewArrayExpression>(loc, std::move(arrayType),
                                                   std::move(size));
  }
  default:
    errorExpectedAnyOf({TT::LParen, TT::LBracket});
  }
}
