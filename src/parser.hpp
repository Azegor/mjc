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

#ifndef PARSER_H
#define PARSER_H

#include <algorithm> // for std::max

#include "ast.hpp"
#include "error.hpp"
#include "input_file.hpp"
#include "lexer.hpp"
#include "util.hpp"

class ParseError : public CompilerError {
public:
  const SourceLocation srcLoc;
  const std::string filename, message, errorLine;
  ParseError(SourceLocation srcLoc, std::string filename, std::string what,
             std::string errorLine)
      : srcLoc(std::move(srcLoc)), filename(std::move(filename)),
        message(std::move(what)), errorLine(errorLine) {}
  const char *what() const noexcept override { return message.c_str(); }

  virtual void writeErrorMessage(std::ostream &out) const override {
    co::color_ostream<std::ostream> cl_out(out);
    cl_out << co::mode(co::bold) << filename << ':' << srcLoc.startToken.line
           << ':' << srcLoc.startToken.col << ": " << co::color(co::red)
           << "error: " << co::color(co::regular) << message << std::endl;
    writeErrorLineHighlight(out);
  }
  void writeErrorLineHighlight(std::ostream &out) const {
    co::color_ostream<std::ostream> cl_out(out);
    size_t highlightStart = srcLoc.startToken.col - 1;
    size_t highlightEnd = srcLoc.endToken.col - 1;
    if (errorLine.length() > Consts::maxErrorLineLength) {
      size_t offset = std::min(highlightStart,
                               errorLine.length() - Consts::maxErrorLineLength);
      // -1 because of leading ellipsis
      if (offset > 0) {
        highlightStart -= (offset - 1);
      }
      highlightEnd =
          std::min(highlightEnd - (offset - 1), Consts::maxErrorLineLength - 2);
      cl_out << truncatedErrorLine(errorLine, offset,
                                   Consts::maxErrorLineLength)
             << std::endl;
    } else {
      cl_out << errorLine << std::endl;
    }
    cl_out << co::color(co::green);

    for (size_t i = 0; i < highlightStart; ++i) {
      cl_out << '~';
    }
    if (srcLoc.startToken.line == srcLoc.endToken.line) {
      for (size_t i = highlightStart; i <= highlightEnd; ++i) {
        cl_out << '^';
      }
    } else {
      cl_out << '^';
    }
    cl_out << std::endl;
  }
};

class Parser {
  const InputFile &inputFile;

  Lexer lexer;
  Token curTok;

public:
  Parser(const InputFile &inputFile) : inputFile(inputFile), lexer(inputFile) {
    readNextToken();
  }

  Token &readNextToken() {
    curTok = lexer.nextToken();
    return curTok;
  }

  Token &lookAhead(size_t numTokens) { return lexer.lookAhead(numTokens); }

  void parseFileOnly();

private:
  void readExpect(Token::Type ttype) {
    readNextToken();
    //     std::cout << curTok.toStr() << std::endl;
    expect(ttype);
  }
  void expectAndNext(Token::Type ttype) {
    expect(ttype);
    readNextToken();
  }
  void expect(Token::Type ttype) {
    if (unlikely(curTok.type != ttype)) {
      error("Unexpected '" + truncateString(curTok.str, 64) + "', expected '" +
            Lexer::getTokenName(ttype) + '\'');
    }
  }
  void expectAny(std::initializer_list<Token::Type> tokens) {
    for (auto &&t : tokens) {
      if (curTok.type == t) {
        return;
      }
    }
    error("Unexpected '" + truncateString(curTok.str, 64) +
          "', expected one of " + listToString(tokens, [](auto t) {
            return "\'"s + Lexer::getTokenName(t) + '\'';
          }));
  }

  [[noreturn]] void error(std::string msg) {
    throw ParseError(curTok, inputFile.getFilename(), std::move(msg),
                     lexer.getCurrentLineFromInput(curTok.pos.line));
  }

  [[noreturn]] void
  errorExpectedAnyOf(std::initializer_list<Token::Type> tokens) {
    error("Unexpected '" + truncateString(curTok.str, 64) +
          "', expected one of " + listToString(tokens, [](auto t) {
            return "\'"s + Lexer::getTokenName(t) + '\'';
          }));
  }

  ast::ProgramPtr parseProgram();
  inline ast::ClassPtr parseClassDeclaration();
  inline void parseClassMember(std::vector<ast::FieldPtr> &fields,
                               std::vector<ast::MethodPtr> &methods,
                               std::vector<ast::MainMethodPtr> &mainMethods);
  inline ast::MainMethodPtr parseMainMethod();
  inline void parseFieldOrMethod(std::vector<ast::FieldPtr> &fields,
                                 std::vector<ast::MethodPtr> &methods);
  inline ast::ParameterList parseParameterList();
  inline ast::ParameterPtr parseParameter();
  inline ast::TypePtr parseType();
  inline ast::BasicTypePtr parseBasicType();
  inline ast::BlockPtr parseBlock();
  inline ast::BlockStmtPtr parseBlockStatement();
  inline ast::StmtPtr parseLocalVarDeclStmt();
  inline ast::StmtPtr parseStmt();
  inline ast::StmtPtr parseIfStmt();
  inline ast::StmtPtr parseReturnStmt();
  inline ast::StmtPtr parseWhileStmt();
  inline ast::StmtPtr parseExprStmt();
  inline ast::ExprPtr parseExpr();
  inline ast::ExprPtr precedenceParse(int minPrec);
  inline ast::ExprPtr parseUnary();
  inline ast::ExprPtr parsePostfixExpr();
  inline ast::ExprPtr parsePrimary();
  inline ast::ExprPtr parseMemberAccess(ast::ExprPtr lhs);
  inline ast::ExprPtr parseArrayAccess(ast::ExprPtr lhs);
  inline ast::ExprList parseArguments();
  inline ast::ExprPtr parseNewExpr();
};

#endif // PARSER_H
