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

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <deque>

using TT = Token::Type;

void Parser::parseFileOnly() { parseProgram(); }

ast::ProgramPtr Parser::parseProgram() {
  std::vector<ast::ClassPtr> classes;
  while (true) {
    switch (curTok.type) {
    case TT::Class:
      classes.emplace_back(parseClassDeclaration());
      break;
    case TT::Eof:
      return std::make_unique<ast::Program>(
          ast::Program({}, std::move(classes)));
    default:
      errorExpectedAnyOf({TT::Class, TT::Eof});
      break;
    }
  }
}

ast::ClassPtr Parser::parseClassDeclaration() {
  std::string name;
  std::vector<ast::FieldPtr> fields;
  std::vector<ast::MethodPtr> methods;
  std::vector<ast::MainMethodPtr> mainMethods;

  expectAndNext(TT::Class);
  name = curTok.str;
  expectAndNext(TT::Identifier);
  expectAndNext(TT::LBrace);
  // parse class members:
  while (true) {
    switch (curTok.type) {
    case TT::RBrace:
      readNextToken();
      return std::make_unique<ast::Class>(SourceLocation{}, std::move(name),
                                          std::move(fields), std::move(methods),
                                          std::move(mainMethods));
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
                              std::vector<ast::MethodPtr> &methods,
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
  expectAndNext(TT::Public);
  expectAndNext(TT::Static);
  expectAndNext(TT::Void);
  // name must be main (check in semantic analysis):
  auto methodName = std::move(curTok.str);
  expectAndNext(TT::Identifier);
  expectAndNext(TT::LParen);
  expect(TT::Identifier);
  if (curTok.str != "String") {
    error("Unexpected '" + curTok.str + "', expected 'String'");
  }
  readNextToken();
  expectAndNext(TT::LBracket);
  expectAndNext(TT::RBracket);
  // name doesn't matter here, but keep for pretty print:
  auto paramName = std::move(curTok.str);
  expectAndNext(TT::Identifier);
  expectAndNext(TT::RParen);
  auto block = parseBlock();
  return std::make_unique<ast::MainMethod>(
      SourceLocation{}, std::move(methodName), std::move(paramName),
      std::move(block));
}

void Parser::parseFieldOrMethod(std::vector<ast::FieldPtr> &fields,
                                std::vector<ast::MethodPtr> &methods) {
  // don't warn on unused
  (void)fields;
  (void)methods;
  expectAndNext(TT::Public);
  parseType();
  expectAndNext(TT::Identifier);
  switch (curTok.type) {
  case TT::Semicolon: // field
    readNextToken();
    return;
  case TT::LParen: // method
    readNextToken();
    parseParameterList();
    expectAndNext(TT::RParen);
    parseBlock();
    return;
  default:
    errorExpectedAnyOf({TT::Semicolon, TT::LParen});
    break;
  }
}

ast::ParameterList Parser::parseParameterList() {
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Identifier:
  case TT::Int:
  case TT::Void:
    parseParameter();
    while (true) {
      switch (curTok.type) {
      case TT::Comma:
        readNextToken();
        parseParameter();
        break;
      default:
        return {}; // TODO
      }
    }
    break;
  default:
    return {}; // TODO
  }
}

ast::ParameterPtr Parser::parseParameter() {
  parseType();
  expectAndNext(TT::Identifier);
  return nullptr; // TODO
}

ast::TypePtr Parser::parseType() {
  parseBasicType();
  while (true) {
    switch (curTok.type) {
    case TT::LBracket:
      readNextToken();
      expectAndNext(TT::RBracket);
      break;
    default:
      return nullptr; // TODO
    }
  }
}

Token::Type Parser::parseBasicType() {
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Identifier:
  case TT::Int:
  case TT::Void: {
    auto type = curTok.type;
    readNextToken();
    return type;
  }
  default:
    errorExpectedAnyOf({TT::Boolean, TT::Identifier, TT::Int, TT::Void});
    break;
  }
}

ast::BlockPtr Parser::parseBlock() {
  expectAndNext(TT::LBrace);
  while (true) {
    switch (curTok.type) {
    case TT::RBrace:
      readNextToken();
      return nullptr; // TODO
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
    case TT::While:
      parseBlockStatement();
      break;
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

ast::StmtPtr Parser::parseLocalVarDeclStmt() {
  parseType();
  expectAndNext(TT::Identifier);
  switch (curTok.type) {
  case TT::Eq:
    readNextToken();
    parseExpr();
    expectAndNext(TT::Semicolon);
    return nullptr; // TODO
  case TT::Semicolon:
    readNextToken();
    return nullptr; // TODO
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
  parseExpr();
  expectAndNext(TT::Semicolon);
  return nullptr; // TODO
}

ast::StmtPtr Parser::parseIfStmt() {
  expectAndNext(TT::If); // necessary?
  expectAndNext(TT::LParen);
  parseExpr();
  expectAndNext(TT::RParen);
  parseStmt();
  if (curTok.type == TT::Else) {
    readNextToken();
    parseStmt();
  }
  return nullptr; // TODO
}

ast::StmtPtr Parser::parseReturnStmt() {
  expectAndNext(TT::Return);
  switch (curTok.type) {
  case TT::Semicolon:
    readNextToken();
    return nullptr; // TODO
  default:
    parseExpr();
    expectAndNext(TT::Semicolon);
    return nullptr; // TODO
  }
}

ast::StmtPtr Parser::parseWhileStmt() {
  expectAndNext(TT::While); // necessary?
  expectAndNext(TT::LParen);
  parseExpr();
  expectAndNext(TT::RParen);
  parseStmt();
  return nullptr; // TODO
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
  parseUnary();
  int opPrec;
  while ((opPrec = getOpPrec(curTok.type)) >= minPrec) {
    auto opTok = std::move(curTok);
    readNextToken();
    if (opTok.type != TT::Eq) { // only right assoc case
      opPrec += 1;
    }
    precedenceParse(opPrec);
    // result = handle_operator(result, rhs);
  }
  return nullptr; // TODO
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
      parsePostfixExpr();
      // result = ...
      // consume all unary prefixes in reverse order
      for (std::deque<Token>::reverse_iterator i = unaries.rbegin(),
                                               end = unaries.rend();
           i != end; ++i) {
        Token t = std::move(*i);
        // consume token in some way
        // TODO implement function which handles unary operators
        // result = UnaryOp(t, result)
      }
      return nullptr; // TODO
    }
  }
}

ast::ExprPtr Parser::parsePostfixExpr() {
  parsePrimary();
  while (true) {
    switch (curTok.type) {
    case TT::Dot:
      parseMemberAccess();
      break;
    case TT::LBracket:
      parseArrayAccess();
      break;
    default:
      return nullptr; // TODO
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
  case TT::False:
    readNextToken();
    return std::make_unique<ast::BoolLiteral>(SourceLocation{}, false);
  case TT::True:
    readNextToken();
    return std::make_unique<ast::BoolLiteral>(SourceLocation{}, true);
  case TT::Null:
    readNextToken();
    return std::make_unique<ast::NullLiteral>(SourceLocation{});
  case TT::This:
    readNextToken();
    return std::make_unique<ast::ThisLiteral>(SourceLocation{});
  case TT::IntLiteral:
    readNextToken();
    return std::make_unique<ast::ThisLiteral>(SourceLocation{});
  case TT::Identifier:
    readNextToken();
    if (curTok.type == TT::LParen) {
      // return "this.methodinvocation(args)
      readNextToken();
      parseArguments();
      expectAndNext(TT::RParen);
    }
    return nullptr; // TODO
  case TT::New:
    return parseNewExpr();
  default:
    errorExpectedAnyOf({TT::LParen, TT::False, TT::True, TT::Null, TT::This,
                        TT::IntLiteral, TT::Identifier, TT::New});
  }
}

ast::ExprPtr Parser::parseMemberAccess() {
  expectAndNext(TT::Dot);
  expectAndNext(TT::Identifier);
  if (curTok.type == TT::LParen) {
    readNextToken();
    parseArguments();
    expectAndNext(TT::RParen);
  }
  return nullptr; // TODO
}

ast::ExprPtr Parser::parseArrayAccess() {
  expectAndNext(TT::LBracket);
  parseExpr();
  expectAndNext(TT::RBracket);
  return nullptr; // TODO
}

ast::ExprList Parser::parseArguments() {
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
    parseExpr();
    while (curTok.type == TT::Comma) {
      readNextToken();
      parseExpr();
    }
    return {}; // TODO
  case TT::RParen:
    return {}; // empty vector
  default:
    errorExpectedAnyOf({TT::Bang, TT::False, TT::Identifier, TT::IntLiteral,
                        TT::New, TT::Null, TT::True, TT::LParen, TT::Minus,
                        TT::This, TT::RParen});
  }
}

ast::ExprPtr Parser::parseNewExpr() {
  expectAndNext(TT::New);
  // check next token instead of current
  switch (lookAhead(1).type) {
  case TT::LParen:
    expectAndNext(TT::Identifier);
    // curTok is always LParen
    readNextToken();
    expectAndNext(TT::RParen);
    return nullptr; // TODO
  case TT::LBracket:
    parseBasicType();
    // curTok is always LBracket
    readNextToken(); // eat '['
    parseExpr();
    expectAndNext(TT::RBracket);
    while (curTok.type == TT::LBracket && lookAhead(1).type == TT::RBracket) {
      readNextToken(); // eat '['
      readNextToken(); // eat ']'
    }
    return nullptr; // TODO
  default:
    errorExpectedAnyOf({TT::LParen, TT::LBracket});
  }
}
