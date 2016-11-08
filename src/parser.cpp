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
  expectAndNext(TT::Identifier);
}

void Parser::parseType() {
  parseBasicType();
  while (true) {
    switch (curTok.type) {
    case TT::LBracket:
      readNextToken();
      expectAndNext(TT::RBracket);
      break;
    default:
      return;
    }
  }
}

void Parser::parseBasicType() {
  switch (curTok.type) {
  case TT::Boolean:
  case TT::Identifier:
  case TT::Int:
  case TT::Void:
    readNextToken();
    break;
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
  case TT::Identifier: {
    Token &nextTok = lookAhead(1);
    Token &afterNextTok = lookAhead(2);
    if (nextTok.type == TT::Identifier ||
        (nextTok.type == TT::LBracket && afterNextTok.type == TT::RBracket)) {
      parseLocalVarDeclStmt();
      return;
    } else {
      parseStmt();
      return;
    }
    break;
  }
  default:
    break;
  }
}

void Parser::parseLocalVarDeclStmt() {
  parseType();
  expectAndNext(TT::Identifier);
  switch (curTok.type) {
  case TT::Eq:
    readNextToken();
    parseExpr();
    expectAndNext(TT::Semicolon);
    break;
  case TT::Semicolon:
    readNextToken();
    break;
  default:
    errorExpectedAnyOf({TT::Eq, TT::Semicolon});
  }
}

// might return nullptr if statement is empty
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
    errorExpectedAnyOf({TT::LBrace, TT::Semicolon, TT::If, TT::Return,
                        TT::While, TT::Bang, TT::False, TT::IntLiteral, TT::New,
                        TT::Null, TT::True, TT::LParen, TT::Minus, TT::This,
                        TT::Identifier});
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
    auto opTok = std::move(curTok);
    readNextToken();
    if (opTok.type != TT::Eq) { // only right assoc case
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

// TODO: use this implementation later when the AST is also constructed
// (since then tail call elimination cannot be applied anymore)
// void Parser::parseUnary() {
//   std::deque<Token> unaries;
//   while (true) {
//     switch (curTok.type) {
//     case TT::Bang:
//       unaries.emplace_back(std::move(curTok));
//       readNextToken();
//       continue; // parseUnary();
//     case TT::Minus:
//       unaries.emplace_back(std::move(curTok));
//       readNextToken();
//       continue; // parseUnary();
//       break;
//     default:
//       parsePostfixExpr();
//       // result = ...
//       // consume all unary prefixes in reverse order
//       for (std::deque<Token>::reverse_iterator i = unaries.rbegin(),
//                                                 end = unaries.rend();
//            i != end; ++i) {
//         Token t = std::move(*i);
//         // consume token in some way
//         // TODO implement function which handles unary operators
//         // result = UnaryOp(t, result)
//       }
//       return;
//     }
//   }
// }

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
      // return "this.methodinvocation(args)
      readNextToken();
      parseArguments();
      expectAndNext(TT::RParen);
    }
    break;
  case TT::New:
    parseNewExpr();
    break;
  default:
    errorExpectedAnyOf({TT::LParen, TT::False, TT::True, TT::Null, TT::This,
                        TT::IntLiteral, TT::Identifier, TT::New});
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
    break;
  case TT::RParen:
    return;
  default:
    errorExpectedAnyOf({TT::Bang, TT::False, TT::Identifier, TT::IntLiteral,
                        TT::New, TT::Null, TT::True, TT::LParen, TT::Minus,
                        TT::This, TT::RParen});
  }
}

void Parser::parseNewExpr() {
  expectAndNext(TT::New);
  // check next token instead of current
  switch (lookAhead(1).type) {
  case TT::LParen:
    expectAndNext(TT::Identifier);
    // curTok is always LParen
    readNextToken();
    expectAndNext(TT::RParen);
    break;
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
    break;
  default:
    expectAny({TT::LParen, TT::LBracket});
    break;
  }
}
