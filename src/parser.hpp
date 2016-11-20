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
#include "symboltable.hpp"
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
           << "error: " << co::reset << message << std::endl;
    srcLoc.writeErrorLineHighlight(out, errorLine, false);
  }
};

class Parser {
  const InputFile &inputFile;
  Lexer lexer;
  SymbolTable::StringTable &strTbl;

  Token curTok;

public:
  Parser(const InputFile &inputFile, SymbolTable::StringTable &strTbl)
      : inputFile(inputFile), lexer(inputFile), strTbl(strTbl) {
    readNextToken();
  }

  Token &readNextToken() {
    curTok = lexer.nextToken();
    return curTok;
  }

  Token &lookAhead(size_t numTokens) { return lexer.lookAhead(numTokens); }

  void parseFileOnly();
  void parseAndPrintAst();
  void parseAndDotAst();

  ast::ProgramPtr parseProgram();

  Lexer& getLexer() { return lexer; }

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
  std::string expectGetIdentAndNext(Token::Type ttype) {
    expect(ttype);
    std::string res = std::move(curTok.str);
    readNextToken();
    return res;
  }
  void expect(Token::Type ttype) {
    if (unlikely(curTok.type != ttype)) {
      std::stringstream errorLine;
      auto cl_err = co::make_colored(errorLine);
      cl_err << "Unexpected " << co::mode(co::bold) << '\''
             << truncateString(curTok.str, 64) << '\'' << co::reset
             << ", expected ";
      if (Lexer::tokenNameNeedsQuotes(ttype)) {
        cl_err << co::mode(co::bold) << '\'' << Lexer::getTokenName(ttype)
               << '\'';
      } else {
        cl_err << co::mode(co::bold) << Lexer::getTokenName(ttype);
      }
      error(errorLine.str());
    }
  }
  void expectAny(std::initializer_list<Token::Type> tokens) {
    for (auto &&t : tokens) {
      if (curTok.type == t) {
        return;
      }
    }
    errorExpectedAnyOf(tokens);
  }

  [[noreturn]] void error(std::string msg) {
    throw ParseError(curTok, inputFile.getFilename(), std::move(msg),
                     lexer.getCurrentLineFromInput(curTok.pos.line));
  }

  [[noreturn]] void
  errorExpectedAnyOf(std::initializer_list<Token::Type> tokens) {
    std::stringstream errorLine;
    auto cl_err = co::make_colored(errorLine);
    cl_err << "Unexpected " << co::mode(co::bold) << '\''
           << truncateString(curTok.str, 64) << '\'' << co::reset
           << ", expected one of " << listToString(tokens, " or ", [](auto t) {
                return "\'"s + Lexer::getTokenName(t) + '\'';
              });

    error(errorLine.str());
  }
  inline ast::ClassPtr parseClassDeclaration();
  inline void parseClassMember(std::vector<ast::FieldPtr> &fields,
                               std::vector<ast::RegularMethodPtr> &methods,
                               std::vector<ast::MainMethodPtr> &mainMethods);
  inline ast::MainMethodPtr parseMainMethod();
  inline void parseFieldOrMethod(std::vector<ast::FieldPtr> &fields,
                                 std::vector<ast::RegularMethodPtr> &methods);
  inline ast::ParameterList parseParameterList();
  inline ast::ParameterPtr parseParameter();
  inline ast::TypePtr parseType();
  inline ast::BasicTypePtr parseBasicType();
  inline ast::BlockPtr parseBlock();
  inline ast::BlockStmtPtr parseBlockStatement();
  inline ast::BlockStmtPtr parseLocalVarDeclStmt();
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
