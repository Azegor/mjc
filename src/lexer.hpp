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

#include <cassert>
#include <cstring> // for std::strerror
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std::string_literals;

#include "error.hpp"
#include "input_file.hpp"
#include "util.hpp"

struct TokenPos {
  int line;
  int col;
  TokenPos() : line(-1), col(-1) {}
  TokenPos(int line, int col) : line(line), col(col) {}
  TokenPos(const TokenPos &) = default;

  bool operator==(const TokenPos &o) const {
    return line == o.line && col == o.col;
  }

  std::string toStr() const {
    return std::to_string(line) + ':' + std::to_string(col);
  }
};

struct SourceLocation {
  const TokenPos startToken, endToken;
  SourceLocation() = default;
  SourceLocation(TokenPos start, TokenPos end)
      : startToken(start), endToken(end) {}
  SourceLocation(const SourceLocation &o)
      : startToken(o.startToken), endToken(o.endToken) {}
  SourceLocation(SourceLocation &&o)
      : startToken(std::move(o.startToken)), endToken(std::move(o.endToken)) {}

  std::string toStr() const {
    if (startToken.col == -1)
      return "at unknown location";
    if (startToken == endToken) // FIXME when does this happen?
      return "at token " + startToken.toStr() + "->" + endToken.toStr();
    return "between token " + startToken.toStr() + " and " + endToken.toStr();
  }

  void writeErrorLineHighlight(std::ostream &out, const std::string errorLine) const {
    co::color_ostream<std::ostream> cl_out(out);
    size_t highlightStart = startToken.col - 1;
    size_t highlightEnd = endToken.col - 1;
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
    if (startToken.line == endToken.line) {
      for (size_t i = highlightStart; i <= highlightEnd; ++i) {
        cl_out << co::mode(co::bold) << '^';
      }
    } else {
      cl_out << co::mode(co::bold) << '^';
    }
    cl_out << std::endl;
  }
};

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
  TokenPos pos;
  std::string str;

  // Operations
  Token() : Token(Type::none, {}, "") {}
  Token(Type type) : Token(type, {}, "<?>") {}

  Token(Type type, TokenPos pos, std::string tokenString)
      : type(type), pos(pos), str(std::move(tokenString)) {}

  Token(const Token &o) : type(o.type), pos(o.pos), str(o.str) {}
  Token(Token &&o) : type(o.type), pos(o.pos), str(std::move(o.str)) {}

  Token &operator=(const Token &o) = default;

  Token &operator=(Token &&o) {
    type = o.type;
    pos = o.pos;
    str = std::move(o.str);
    return *this;
  }

  bool operator==(const Token &o) const {
    return type == o.type && pos == o.pos && str == o.str;
  }

  std::string toStr() const {
    return '<' + str + " at " + std::to_string(pos.line) + ':' +
           std::to_string(pos.col) + '>';
  }

  TokenPos startPos() const { return pos; }
  TokenPos endPos() const {
    TokenPos tmp = pos;
    tmp.col += str.length() - 1;
    return tmp;
  }
  SourceLocation singleTokenSrcLoc() const { return {startPos(), endPos()}; }
  operator SourceLocation() const { return singleTokenSrcLoc(); }

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

class LexError : public CompilerError {
public:
  const int line, col;
  const std::string filename, message, errorLine;
  LexError(std::string file, int line, int col, std::string what,
           std::string errorLine)
      : line(line), col(col), filename(file), message(std::move(what)),
        errorLine(std::move(errorLine)) {}
  const char *what() const noexcept override { return message.c_str(); }

  virtual void writeErrorMessage(std::ostream &out) const override {
    co::color_ostream<std::ostream> cl_out(out);
    cl_out << co::mode(co::bold) << filename << ':' << line << ':' << col
           << ": " << co::color(co::red) << "error: " << co::color(co::regular)
           << message << std::endl;
    writeErrorLineHighlight(out);
  }

  void writeErrorLineHighlight(std::ostream &out) const {
    co::color_ostream<std::ostream> cl_out(out);
    size_t highlightPos = col;
    if (highlightPos > Consts::maxErrorLineLength) {
      highlightPos = Consts::maxErrorLineLength - 8;
      cl_out << truncatedErrorLine(errorLine, col - highlightPos + 1,
                                   Consts::maxErrorLineLength)
             << std::endl;
    } else {
      cl_out << errorLine << std::endl;
    }
    cl_out << co::color(co::green);
    for (size_t i = 1; i < highlightPos; ++i)
      cl_out << '~';
    cl_out << '^' << std::endl;
  }
};

class Lexer {
  static std::unordered_map<std::string, Token::Type> identifierTokens;

  std::istream &input;
  std::string filename;

  int lastChar;
  // 1024 seems to be the sweet spot:
  static constexpr const int maxBufferSize = 1024;
  // input stream buffer
  char buffer[maxBufferSize];
  int nextCharPos = 0;
  int curBufferSize = 0;
  bool streamIsEof = false;
  std::streamoff bufferStartOffset = 0;

  std::string tokenString;
  int line = 1, column = 0; // column is 0 since constructor calls nextChar()
  int tokenLine, tokenCol;
  std::vector<std::streamoff> lineStartFileOffsets;

  std::deque<Token> tokenLookahead;

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
    return Token(type, {tokenLine, tokenCol}, tokenString);
  }

  [[noreturn]] void error(std::string msg) {
    throw LexError(filename, line, column, std::move(msg),
                   getCurrentLineFromInput(line));
  }

  [[noreturn]] void invalidCharError(int errorChar) {
    if (std::isprint(errorChar)) {
      error("Invalid input character: '"s + (char)errorChar + "'");
    }
    std::stringstream invalidChar;
    invalidChar << "\\0x" << std::hex << (int)((unsigned char)errorChar);
    error("Invalid input character: '"s + invalidChar.str() + "'");
  }

public:
  void readIntoBuffer() {
    bufferStartOffset = input.tellg();
    input.read(&buffer[0], maxBufferSize);
    streamIsEof = input.eof();
    curBufferSize = input.gcount();
    nextCharPos = 0;
    if (streamIsEof) {
      buffer[curBufferSize] = EOF;
    }
  }

  bool isEof() const {
    return streamIsEof && (nextCharPos - 1 == curBufferSize);
  }

  Lexer(const InputFile &inputFile)
      : input(*inputFile.getStream()), filename(inputFile.getFilename()) {
    if (!input) {
      error(std::string("Broken input stream: ") + std::strerror(errno));
    }
    lineStartFileOffsets.push_back(0);
    readIntoBuffer();
    nextChar();
  }

  std::string getCurrentLineFromInput(int lineNr);

  Token nextToken() {
    if (tokenLookahead.empty()) {
      return readNextToken();
    }
    Token res = std::move(tokenLookahead.front());
    tokenLookahead.pop_front();
    return res;
  }

  Token &lookAhead(size_t numTokens) {
    assert(numTokens > 0);
    if (numTokens <= tokenLookahead.size()) {
      return tokenLookahead[numTokens - 1];
    } else {
      return peekAhead(numTokens);
    }
  }

  const std::string& getFilename() const { return filename; }

  static bool tokenNameNeedsQuotes(Token::Type type);
  static const char *getTokenName(Token::Type type);

private:
  Token &peekAhead(size_t numTokens) {
    for (size_t i = tokenLookahead.size(); i <= numTokens; ++i) {
      tokenLookahead.push_back(readNextToken());
    }
    return tokenLookahead[numTokens - 1];
  }
  Token readNextToken();

  // Parser helper functions
  inline bool skipCommentsAndWhitespaces();
  inline Token readSlashFromSecondCharOn();
  inline Token readDecNumber();
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
