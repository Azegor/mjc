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

#include <string>
using namespace std::string_literals;

std::unordered_map<std::string, Token::Type> Lexer::identifierTokens{
    // keywords
    {"boolean", Token::Type::Boolean},
    {"class", Token::Type::Class},
    {"else", Token::Type::Else},
    {"false", Token::Type::False},
    {"if", Token::Type::If},
    {"int", Token::Type::Int},
    {"new", Token::Type::New},
    {"null", Token::Type::Null},
    {"public", Token::Type::Public},
    {"return", Token::Type::Return},
    {"static", Token::Type::Static},
    {"this", Token::Type::This},
    {"true", Token::Type::True},
    {"void", Token::Type::Void},
    {"while", Token::Type::While},

    // reserved but unused keywords:
    {"abstract", Token::Type::ReservedKeyword},
    {"assert", Token::Type::ReservedKeyword},
    {"break", Token::Type::ReservedKeyword},
    {"byte", Token::Type::ReservedKeyword},
    {"case", Token::Type::ReservedKeyword},
    {"catch", Token::Type::ReservedKeyword},
    {"char", Token::Type::ReservedKeyword},
    {"const", Token::Type::ReservedKeyword},
    {"continue", Token::Type::ReservedKeyword},
    {"default", Token::Type::ReservedKeyword},
    {"double", Token::Type::ReservedKeyword},
    {"do", Token::Type::ReservedKeyword},
    {"enum", Token::Type::ReservedKeyword},
    {"extends", Token::Type::ReservedKeyword},
    {"finally", Token::Type::ReservedKeyword},
    {"final", Token::Type::ReservedKeyword},
    {"float", Token::Type::ReservedKeyword},
    {"for", Token::Type::ReservedKeyword},
    {"goto", Token::Type::ReservedKeyword},
    {"implements", Token::Type::ReservedKeyword},
    {"import", Token::Type::ReservedKeyword},
    {"instanceof", Token::Type::ReservedKeyword},
    {"interface", Token::Type::ReservedKeyword},
    {"long", Token::Type::ReservedKeyword},
    {"native", Token::Type::ReservedKeyword},
    {"package", Token::Type::ReservedKeyword},
    {"private", Token::Type::ReservedKeyword},
    {"protected", Token::Type::ReservedKeyword},
    {"short", Token::Type::ReservedKeyword},
    {"strictfp", Token::Type::ReservedKeyword},
    {"super", Token::Type::ReservedKeyword},
    {"switch", Token::Type::ReservedKeyword},
    {"synchronized", Token::Type::ReservedKeyword},
    {"throws", Token::Type::ReservedKeyword},
    {"throw", Token::Type::ReservedKeyword},
    {"transient", Token::Type::ReservedKeyword},
    {"try", Token::Type::ReservedKeyword},
    {"volatile", Token::Type::ReservedKeyword},
};

Token::Type Lexer::getSingleCharOpToken(int c) {
  switch (c) {
  case '(':
    return Token::Type::LParen;
  case ')':
    return Token::Type::RParen;
  case '[':
    return Token::Type::LBracket;
  case ']':
    return Token::Type::RBracket;
  case '{':
    return Token::Type::LBrace;
  case '}':
    return Token::Type::RBrace;
  case '?':
    return Token::Type::Questionmark;
  case ',':
    return Token::Type::Comma;
  case '.':
    return Token::Type::Dot;
  case ':':
    return Token::Type::Colon;
  case ';':
    return Token::Type::Semicolon;
  case '~':
    return Token::Type::Tilde;
  default:
    return Token::Type::none;
  }
}

int Lexer::nextChar() {
  static enum { normal, cr, lf, crlf } lineState = normal;

  lastChar = input.get();

  // the following is to identify different line breaks correctly
  // (mixture of CR and LF)
  switch (lineState) {
  case normal: {
    switch (lastChar) {
    case '\r':
      lineState = cr;
      break;
    case '\n':
      lineState = lf;
      break;
    default:
      // lineState = normal;
      break;
    }
    break;
  }
  case cr: {
    switch (lastChar) {
    case '\r':
      // lineState = cr;
      goto newline;
    case '\n':
      lineState = crlf;
      break;
    default:
      lineState = normal;
      goto newline;
    }
    break;
  }
  case lf: {
    switch (lastChar) {
    case '\r':
      lineState = cr;
      goto newline;
    case '\n':
      // lineState = lf;
      goto newline;
    default:
      lineState = normal;
      goto newline;
    }
    break;
  }
  case crlf: {
    switch (lastChar) {
    case '\r':
      lineState = cr;
      goto newline;
    case '\n':
      lineState = lf;
      goto newline;
    default:
      lineState = normal;
      goto newline;
    }
    break;
  }
  newline:
    lines.emplace_back();
    currentLine = &lines.back();
    if (lastChar != '\r' && lastChar != '\n' && !input.eof())
      *currentLine += lastChar;
    column = 1;
    ++line;
    return lastChar;
  } // switch end

  if (lastChar != '\r' && lastChar != '\n' && !input.eof())
    *currentLine += lastChar;
  ++column;
  return lastChar;
}

Token Lexer::nextToken() {
  // skip all whitespaces
  while (std::isspace(lastChar))
    nextChar();

  initToken(); // clear tokenstring, sets line/col to current pos

  // identifiers [_a-zA-Z][_a-zA-Z0-9]*
  if (std::isalpha(lastChar) || lastChar == '_') {
    tokenString = lastChar;
    while (std::isalnum(nextChar()) || lastChar == '_')
      tokenString += lastChar;

    auto res = identifierTokens.find(tokenString);
    if (res != identifierTokens.end())
      return makeToken(res->second);
    return makeToken(Token::Type::Identifier);
  }

  // leading 0:
  if (lastChar == '0')
    return readLeadingZeroNumber();

  // leading 1-9:
  if (std::isdigit(lastChar))
    return readDecNumber();

  //-----------
  // operators (single or multiple characters)
  if (lastChar == '/')
    return readSlash();

  if (lastChar == '*')
    return readStar();

  if (lastChar == '+')
    return readPlus();

  if (lastChar == '-')
    return readMinus();

  if (lastChar == '<')
    return readLT();

  if (lastChar == '>')
    return readGT();

  if (lastChar == '&')
    return readAnd();

  if (lastChar == '|')
    return readOr();

  // seems to not exist as '~=' variant
  //   if (lastChar == '~')
  //     return readTilde();

  if (lastChar == '^')
    return readCarret();

  if (lastChar == '=')
    return readEq();

  if (lastChar == '!')
    return readBang();

  // -----------
  // end of file
  if (input.eof())
    return makeToken(Token::Type::Eof);

  // remaining single characters as tokens (i.e. operator symbols)
  tokenString = lastChar;
  int thisChar = lastChar;
  nextChar(); // eat
  Token::Type type = getSingleCharOpToken(lastChar);
  if (type == Token::Type::none) { // illegal character
    error("Invalid input character: '"s + (char)lastChar + "'");
  }
  return makeToken(type);
}

Token Lexer::readLeadingZeroNumber() {}
Token Lexer::readDecNumber() {}
Token Lexer::readSlash() {}
Token Lexer::readStar() {}
Token Lexer::readPlus() {}
Token Lexer::readMinus() {}
Token Lexer::readLT() {}
Token Lexer::readGT() {}
Token Lexer::readAnd() {}
Token Lexer::readOr() {}
// Token Lexer::readTilde() {}
Token Lexer::readCarret() {}
Token Lexer::readEq() {}
Token Lexer::readBang() {}
