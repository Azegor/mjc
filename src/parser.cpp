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

#include "lexer.hpp"
#include "parser.hpp"

using TT = Token::Type;

void Parser::parseFileOnly() { parseProgram(); }

void Parser::parseProgram() {
  readNextToken();
  while (true) {
    switch (curTok.type) {
    case TT::Class:
      parseClassDeclaration();
      break;
    case TT::Eof:
      return;
    default:
      expectAny({TT::Class, TT::Eof});
    }
  }
}

void Parser::parseClassDeclaration() {
  readExpect(TT::Identifier);
  readExpect(TT::LBrace);
  // parse class members
  readNextToken();
  while (true) {
    switch (curTok.type) {
    case TT::RBrace:
      readNextToken();
      return;
    case TT::Public:
      parseClassMember();
      break;
    default:
      expectAny({TT::RBrace, TT::Public});
    }
  }
}

void Parser::parseClassMember() {
  switch (readNextToken().type) {
  case TT::Static:
    parseMainMethod();
    return;
  case TT::Boolean:
  case TT::Identifier:
  case TT::Int:
  case TT::Void:
    parseFieldOrMethod();
    return;
  default:
    expectAny({TT::Static, TT::Boolean, TT::Identifier, TT::Int, TT::Void});
  }
}

void Parser::parseMainMethod() {
  readExpect(TT::Void);
  readExpect(TT::Identifier); // name doesn't matter here
  readExpect(TT::LParen);
  readExpect(TT::Identifier);
  if (curTok.str != "String") {
    error("Unexpected '" + curTok.str + "', expected 'String'");
  }
  readExpect(TT::LBracket);
  readExpect(TT::RBracket);
  readExpect(TT::Identifier);
  readExpect(TT::RParen);
  readNextToken();
  parseBlock();
}

void Parser::parseFieldOrMethod() {
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
    expectAny({TT::Semicolon, TT::LParen});
  }
}

void Parser::parseParameterList() {
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
        return;
      }
    }
    break;
  default:
    return;
  }
}

void Parser::parseParameter() {
  parseType();
  expect(TT::Identifier);
  readNextToken();
}

void Parser::parseType() {
  parseBasicType();
  while (true) {
    switch (curTok.type) {
    case TT::LBracket:
      readExpect(TT::RBracket);
      readNextToken();
      break;
    default:
      return;
    }
  }
}

void Parser::parseBasicType() {
  expectAny({TT::Boolean, TT::Identifier, TT::Int, TT::Void});
  readNextToken();
}

void Parser::parseBlock() {
  expect(TT::LBrace);
  readNextToken();
  while (true) {
    switch (curTok.type) {
    case TT::RBrace:
      readNextToken();
      return;
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
      expectAny({TT::RBrace, TT::Bang, TT::Boolean, TT::False, TT::Identifier,
                 TT::If, TT::Int, TT::IntLiteral, TT::New, TT::Null, TT::True,
                 TT::LBrace, TT::LParen, TT::Minus, TT::Return, TT::Semicolon,
                 TT::This, TT::Void, TT::While});
    }
  }
}

void Parser::parseBlockStatement() {
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Int:
  case TT::Void:
    parseLocalVarDeclStmt();
    break;
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
    parseStmt();
  case TT::Identifier:
    switch (nextTok.type) {
    case TT::Identifier:
      parseLocalVarDeclStmt();
      return;
    default:
      parseStmt();
    }
  default:
    break;
  }
}

void Parser::parseLocalVarDeclStmt() {
  parseType();
  expect(TT::Identifier);
  if (readNextToken().type == TT::Eq) {
    readNextToken();
    parseExpr();
  }
  expect(TT::Semicolon);
  readNextToken();
}

void Parser::parseStmt() {
  switch (curTok.type) {
  case TT::LBrace:
    parseBlock();
    break;
  case TT::Semicolon:
    readNextToken(); // empty statement -> ignore
    break;
  case TT::If:
    parseIfStmt();
    break;
  case TT::Return:
    parseReturnStmt();
    break;
  case TT::While:
    parseWhileStmt();
    break;
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
    parseExprStmt();
    break;
  default:
    expectAny({TT::LBrace, TT::Semicolon, TT::If, TT::Return, TT::While,
               TT::Bang, TT::False, TT::IntLiteral, TT::New, TT::Null, TT::True,
               TT::LParen, TT::Minus, TT::This, TT::Identifier});
    break;
  }
}

void Parser::parseExprStmt() {
  parseExpr();
  expectAndNext(TT::Semicolon);
}

void Parser::parseIfStmt() {
  expectAndNext(TT::If); // necessary?
  expectAndNext(TT::LParen);
  parseExpr();
  expectAndNext(TT::RParen);
  parseStmt();
  if (curTok.type == TT::Else) {
    readNextToken();
    parseStmt();
  }
}

void Parser::parseReturnStmt() {
  readNextToken(); // TODO
}

void Parser::parseWhileStmt() {
  readNextToken(); // TODO
}

void Parser::parseExpr() {
  readNextToken(); // TODO
}
