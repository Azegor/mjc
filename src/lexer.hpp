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

#ifndef LEXER_H
#define LEXER_H

#include <cstring>  // for std::strerror
#include <iostream> // TODO remove
#include <string>
#include <unordered_map>
#include <vector>
using namespace std::string_literals;

#include "input_file.hpp"

struct Token {
  enum class Type : int {
    none = 0x0,
    Eof,

    Identifier,
    IntLiteral,

    // -- keywords:
    Boolean,
    Class,
    Else,
    False,
    If,
    Int,
    New,
    Null,
    Public,
    Return,
    Static,
    This,
    True,
    Void,
    While,

    // -- operators:
    Amp,
    AmpAmp,
    AmpEq,
    Bang,
    BangEq,
    Caret,
    CaretEq,
    Colon,
    Comma,
    Dot,
    Eq,
    EqEq,
    Gt,
    GtEq,
    GtGt,
    GtGtEq,
    GtGtGt,
    GtGtGtEq,
    LBrace,
    LBracket,
    LParen,
    LtEq,
    LtLt,
    Lt,
    LtLtEq,
    Minus,
    MinusEq,
    MinusMinus,
    VBar,
    VBarEq,
    VBarVBar,
    Percent,
    PercentEq,
    Plus,
    PlusEq,
    PlusPlus,
    QMark,
    RBrace,
    RBracket,
    RParen,
    Semicolon,
    Slash,
    SlashEq,
    Star,
    StarEq,
    Tilde,

    ReservedKeyword,
  };

  // Members
  Type type;
  int line, col;
  std::string str;

  // Operations
  Token() : Token(Type::none, 0, 0, "") {}
  Token(Type type) : Token(type, 0, 0, "<?>") {}

  Token(Type type, int line, int col, std::string tokenString)
      : type(type), line(line), col(col), str(std::move(tokenString)) {}

  Token(const Token &o) : type(o.type), line(o.line), col(o.col), str(o.str) {}
  Token(Token &&o)
      : type(o.type), line(o.line), col(o.col), str(std::move(o.str)) {}

  Token &operator=(const Token &o) = default;

  Token &operator=(Token &&o) {
    type = o.type;
    line = o.line;
    col = o.col;
    str = std::move(o.str);
    return *this;
  }

  bool operator==(const Token &o) const {
    return type == o.type && line == o.line && col == o.col && str == o.str;
  }

  std::string toStr() const {
    return '<' + str + " at " + std::to_string(line) + ':' +
           std::to_string(col) + '>';
  }

  friend std::ostream &operator<<(std::ostream &o, const Token &t) {
    switch (t.type) {
    case Type::Identifier:
      o << "identifier " << t.str;
      break;
    case Type::IntLiteral:
      o << "integer literal " << t.str;
      break;
    default:
      o << t.str;
      break;
    }
    return o;
  }
};

class Lexer {
  static std::unordered_map<std::string, Token::Type> identifierTokens;

  std::istream &input;
  std::string filename;

  int lastChar;
  std::string tokenString;
  int line = 1, column = 0; // column is 0 since constructor calls nextChar()
  int tokenLine, tokenCol;
  int currentLineFileOffset = 0;

  Token::Type getSingleCharOpToken(int c);

  int nextChar();
  void appendAndNext() {
    tokenString += lastChar;
    nextChar();
  }

  void initToken() {
    tokenString.clear();
    tokenLine = line;
    tokenCol = column;
  }

  Token makeToken(Token::Type type) {
    // implicit: make a copy of 'tokenString' to keep the string's buffer
    return Token(type, tokenLine, tokenCol, tokenString);
  }

  [[noreturn]] void error(std::string msg) {
    int _line = line, _column = column;
    throw LexError(filename, _line, _column, std::move(msg),
                   getCurrentLineFromInput());
  }
  [[noreturn]] void errorAtTokenStart(std::string msg) {
    throw LexError(filename, tokenLine, tokenCol, std::move(msg),
                   getCurrentLineFromInput());
  }

  [[noreturn]] void invalidCharError(char errorChar) {
    if (std::isspace(errorChar) || std::isprint(errorChar)) {
      errorAtTokenStart("Invalid input character: '"s + errorChar + "'");
    }
    std::stringstream invalidChar;
    invalidChar << "\\0x" << std::hex << errorChar;
    errorAtTokenStart("Invalid input character: '"s + invalidChar.str() + "'");
  }

  std::string getCurrentLineFromInput() {
    int oldPos = input.tellg();
    input.seekg(currentLineFileOffset);
    std::string res;
    int c = input.get();
    constexpr int maxLineLength = 1024;
    int lineLength = 0;
    while (c != '\r' && c != '\n' && input.good() &&
           ++lineLength < maxLineLength) {
      res.push_back(c);
      c = input.get();
    }
    if (lineLength == maxLineLength) {
      res += "...";
    }
    input.seekg(oldPos);
    return res.empty() ? co::color_output(co::cyan, co::normal)("\\empty-line")
                       : res;
  }

public:
  Lexer(const InputFile &inputFile)
      : input(*inputFile.getStream()), filename(inputFile.getFilename()) {
    if (!input) {
      error(std::string("Broken input stream: ") + std::strerror(errno));
    }
    nextChar();
  }

  Token nextToken();

private:
  // Parser helper functions
  inline Token readLeadingZeroNumber();
  inline Token readDecNumber();
  inline Token readSlash();
  inline Token readStar();
  inline Token readPercent();
  inline Token readPlus();
  inline Token readMinus();
  inline Token readLT();
  inline Token readGT();
  inline Token readEq();
  inline Token readBang();
  inline Token readAmp();
  inline Token readVBar();
  //  inline Token readTilde(); // seems to not exist as '~=' variant
  inline Token readCaret();
};

#endif // LEXER_H
