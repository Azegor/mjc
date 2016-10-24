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

#include "input_file.hpp"

struct Token {
  enum class Type : int {
    none = 0x0,
    eof,
    identifier,
    integer,
    // ...
  };

  // Members
  Type type;
  int line, col;
  std::string str;

  // Operations
  Token() : Token(Type::none, 0, 0, "") {}
  Token(Type type) : Token(type, 0, 0, "") {}

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
    case Type::identifier:
      o << "identifier " << t.str;
      break;
    case Type::integer:
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
  const InputFile &inputFile;

public:
  Lexer(const InputFile &inputFile) : inputFile(inputFile) {}

  Token nextToken();
};

#endif // LEXER_H
