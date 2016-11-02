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
  while (true) {
    switch (curTok.type) {
    case TT::Class:
      parseClassDeclaration();
      break;
    case TT::Eof:
      return;
    default:
      expectAny({TT::Class, TT::Eof});
      break;
    }
  }
}

void Parser::parseClassDeclaration() {
  // eat class
  readExpect(TT::Identifier);
  readExpect(TT::LBrace);
  // parse class members:
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
      break;
    }
  }
}

void Parser::parseClassMember() {
  // eat public
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
    break;
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
    break;
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
  expectAndNext(TT::LBrace);
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
      break;
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
    break;
   case TT::Identifier:
    if (nextTok.type == TT::Identifier || 
        (nextTok.type == TT::LBracket && afternextTok.type == TT::RBracket)) {
      parseLocalVarDeclStmt();
      return;
    } else {
      parseStmt();
      return;
    }
    break;
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
  expectAndNext(TT::Return);
  switch (curTok.type) {
  case TT::Semicolon:
    readNextToken();
    return;
  default:
    parseExpr();
    expectAndNext(TT::Semicolon);
    return;
  }
}

void Parser::parseWhileStmt() {
  expectAndNext(TT::While); // necessary?
  expectAndNext(TT::LParen);
  parseExpr();
  expectAndNext(TT::RParen);
  parseStmt();
}

void Parser::parseExpr() { precedenceParse(0); }

enum class Assoc : int8_t {
  Left = 0,
  Right,
  None,
};

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

void Parser::precedenceParse(int minPrec) {
  // check precondition!
  parseUnary();
  int opPrec;
  while ((opPrec = getOpPrec(curTok.type)) >= minPrec) {
    readNextToken();
    if (curTok.type != TT::Eq) { // only right assoc case
      opPrec += 1;
    }
    precedenceParse(opPrec);
    // result = handle_operator(result, rhs);
  }
}

void Parser::parseUnary() {
  switch (curTok.type) {
  case TT::Bang:
    readNextToken();
    parseUnary();
    break;
  case TT::Minus:
    readNextToken();
    parseUnary();
    break;
  default:
    parsePostfixExpr();
    break;
  }
}

void Parser::parsePostfixExpr() {
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
      return;
    }
  }
}

void Parser::parsePrimary() {
  switch (curTok.type) {
  case TT::LParen:
    readNextToken();
    parseExpr();
    expectAndNext(TT::RParen);
    break;
  case TT::False:
    readNextToken();
    break;
  case TT::True:
    readNextToken();
    break;
  case TT::Null:
    readNextToken();
    break;
  case TT::This:
    readNextToken();
    break;
  case TT::IntLiteral:
    readNextToken();
    break;
  case TT::Identifier:
    readNextToken();
    if (curTok.type == TT::LParen) {
      readNextToken();
      parseArguments();
      expectAndNext(TT::RParen);
    }
    break;
  case TT::New:
    parseNewExpr();
    break;
  default:
    error("Unexpected '" + curTok.str + "', expected PrimaryExpression");
  }
}

void Parser::parseMemberAccess() {
  expectAndNext(TT::Dot);
  expectAndNext(TT::Identifier);
  if (curTok.type == TT::LParen) {
    readNextToken();
    parseArguments();
    expectAndNext(TT::RParen);
  }
}

void Parser::parseArrayAccess() {
  expectAndNext(TT::LBracket);
  parseExpr();
  expectAndNext(TT::RBracket);
}

void Parser::parseArguments() {
  while (true) {
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
      break;
    default:
      return;
    }
  }
}

void Parser::parseNewExpr() {
  readNextToken(); // eat new
  switch (nextTok.type) {
  case TT::LParen:
    expectAndNext(TT::Identifier);
    // curTok is always LParen
    readNextToken();
    expectAndNext(TT::RParen);
    break;
  case TT::LBracket:
    parseBasicType();
    // curTok is always LBracket
    readNextToken();
    parseExpr();
    expectAndNext(TT::RBracket);
    while (curTok.type == TT::LBracket && nextTok.type == TT::RBracket) {
      // eat 2x
      readNextToken();
      readNextToken();
    }
    break;
  default:
    expectAny({TT::LParen, TT::LBracket});
    break;
  }
}
