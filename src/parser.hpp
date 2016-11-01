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
    cl_out << errorLine << std::endl;
    cl_out << co::color(co::green);

    for (int i = 1; i < srcLoc.startToken.col; ++i)
      cl_out << '~';
    if (srcLoc.startToken.line == srcLoc.endToken.line) {
      for (int i = srcLoc.startToken.col; i <= srcLoc.endToken.col; ++i)
        cl_out << '^';
    } else {
      cl_out << '^';
    }
    cl_out << std::endl;
  }
};

class Parser {
  const InputFile &inputFile;

  Lexer lexer;
  Token curTok, nextTok;

public:
  Parser(const InputFile &inputFile) : inputFile(inputFile), lexer(inputFile) {
    // 2x
    readNextToken();
    readNextToken();
  }

  Token &readNextToken() {
    curTok = std::move(nextTok);
    nextTok = lexer.nextToken();
    return curTok;
  }

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
    if (curTok.type != ttype)
      error("Unexpected '" + curTok.str + "', expected '" +
            Lexer::getTokenName(ttype) + '\'');
  }
  void expectAny(std::initializer_list<Token::Type> tokens) {
    for (auto &&t : tokens) {
      if (curTok.type == t) {
        return;
      }
    }
    error("Unexpected '" + curTok.str + "', expected one of " +
          listToString(tokens, [](auto t) {
            return "\'"s + Lexer::getTokenName(t) + '\'';
          }));
  }

  [[noreturn]] void error(std::string msg) {
    throw ParseError(curTok, inputFile.getFilename(), std::move(msg),
                     lexer.getCurrentLineFromInput());
  }

  void parseProgram();
  void parseClassDeclaration();
  void parseClassMember();
  void parseMainMethod();
  void parseFieldOrMethod();
  void parseParameterList();
  void parseParameter();
  void parseType();
  void parseBasicType();
  void parseBlock();
  void parseBlockStatement();
  void parseLocalVarDeclStmt();
  void parseStmt();
  void parseIfStmt();
  void parseReturnStmt();
  void parseWhileStmt();
  void parseExprStmt();
  void parseExpr();
  void precedenceParse(int minPrec);
  void parseUnary();
  void parsePostfixExpr();
  void parsePrimary();
  void parseMemberAccess();
  void parseArrayAccess();
  void parseArguments();
  void parseNewExpr();
};

#endif // PARSER_H
