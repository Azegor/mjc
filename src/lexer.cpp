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
    return Token::Type::QMark;
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
    column = 1;
    ++line;

    currentLineFileOffset = input.tellg() - static_cast<std::streamoff>(1);

    return lastChar;
  } // switch end

  ++column;
  return lastChar;
}

static bool isSpace(int c) {
  switch (c) {
  case ' ':
  case '\r':
  case '\n':
  case '\t':
    return true;
  default:
    return false;
  }
}

Token Lexer::nextToken() {
  // skip all whitespaces
  while (isSpace(lastChar)) {
    nextChar();
  }

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
  switch (lastChar) {
  case '/':
    return readSlash();
  case '*':
    return readStar();
  case '%':
    return readPercent();
  case '+':
    return readPlus();
  case '-':
    return readMinus();
  case '<':
    return readLT();
  case '>':
    return readGT();
  case '&':
    return readAmp();
  case '|':
    return readVBar();
  // seems to not exist as '~=' variant
  // case '~':
  //   return readTilde();
  case '^':
    return readCaret();
  case '=':
    return readEq();
  case '!':
    return readBang();
  default:
    break;
  }

  // -----------
  // end of file
  if (input.eof()) {
    tokenString = "EOF";
    return makeToken(Token::Type::Eof);
  }

  // remaining single characters as tokens (i.e. operator symbols)
  tokenString = lastChar;
  Token::Type type = getSingleCharOpToken(lastChar);
  if (type == Token::Type::none) { // illegal character
    if (std::isspace(lastChar) || std::isprint(lastChar)) {
      errorAtTokenStart("Invalid input character: '"s + (char)lastChar + "'");
    }
    std::stringstream invalidChar;
    invalidChar << "\\0x" << std::hex << lastChar;
    errorAtTokenStart("Invalid input character: '"s + invalidChar.str() + "'");
  }
  nextChar(); // eat
  return makeToken(type);
}

Token Lexer::readLeadingZeroNumber() { // read '0'
  tokenString = '0';
  nextChar();
  if ('0' <= lastChar && lastChar <= '9') {
    errorAtTokenStart("Invalid number with leading zero");
  }
  return makeToken(Token::Type::IntLiteral);
}

Token Lexer::readDecNumber() { // read '[1-9][0-9]*'
  tokenString = lastChar;
  while (std::isdigit(nextChar()))
    tokenString += lastChar;
  return makeToken(Token::Type::IntLiteral);
}

Token Lexer::readSlash() { // read '/' '/=' '//' '/*'
  tokenString = '/';
  if (nextChar() == '/') { // single line comment
    // read until line break or EOF
    do {
      nextChar();
    } while (lastChar != '\r' && lastChar != '\n' && !input.eof());
    // eat line break
    if (!input.eof()) {
      if (lastChar == '\n' || (lastChar == '\r' && nextChar() == '\n')) {
        nextChar(); // eat '\n'
      }
      if (!input.eof()) {
        return nextToken(); // recursive tail call?
      }
    }
    tokenString = "EOF";
    return makeToken(Token::Type::Eof);
  } else if (lastChar == '*') { // multi line comment
    nextChar();
    while (!input.eof()) {
      if (lastChar == '*') {
        if (nextChar() == '/') {
          nextChar();         // eat '/'
          return nextToken(); // recursive tail call?
        } else
          continue; // skip call to nextChar() for cases like '**/'
      }
      nextChar();
    }
    error("unexpected end of file in multi line comment");
  } else if (lastChar == '=') {
    appendAndNext();
    return makeToken(Token::Type::SlashEq);
  } else {
    return makeToken(Token::Type::Slash);
  }
}

Token Lexer::readStar() { // read '*' '*='
  tokenString = '*';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::StarEq);
  }
  return makeToken(Token::Type::Star);
}

Token Lexer::readPercent() { // read '%' '%='
  tokenString = '%';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::PercentEq);
  }
  return makeToken(Token::Type::Percent);
}

Token Lexer::readPlus() { // read '+' '++' '+='
  tokenString = '+';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::PlusEq);
  }
  if (lastChar == '+') {
    appendAndNext();
    return makeToken(Token::Type::PlusPlus);
  }
  return makeToken(Token::Type::Plus);
}

Token Lexer::readMinus() { // read '-' '--' '-='
  tokenString = '-';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::MinusEq);
  }
  if (lastChar == '-') {
    appendAndNext();
    return makeToken(Token::Type::MinusMinus);
  }
  return makeToken(Token::Type::Minus);
}

Token Lexer::readLT() { // read '<' '<=' '<<' '<<='
  tokenString = '<';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::LtEq);
  }
  if (lastChar == '<') {
    appendAndNext();
    if (lastChar == '=') {
      appendAndNext();
      return makeToken(Token::Type::LtLtEq);
    }
    return makeToken(Token::Type::LtLt);
  }
  return makeToken(Token::Type::Lt);
}

Token Lexer::readGT() { // read '>' '>=' '>>' '>>=' '>>>' '>>>='
  tokenString = '>';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::GtEq);
  }
  if (lastChar == '>') {
    appendAndNext();
    if (lastChar == '=') {
      appendAndNext();
      return makeToken(Token::Type::GtGtEq);
    }
    if (lastChar == '>') {
      appendAndNext();
      if (lastChar == '=') {
        appendAndNext();
        return makeToken(Token::Type::GtGtGtEq);
      }
      return makeToken(Token::Type::GtGtGt);
    }
    return makeToken(Token::Type::GtGt);
  }
  return makeToken(Token::Type::Gt);
}

Token Lexer::readEq() { // read '=' '=='
  tokenString = '=';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::EqEq);
  }
  return makeToken(Token::Type::Eq);
}

Token Lexer::readBang() { // read '!' '!='
  tokenString = '!';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::BangEq);
  }
  return makeToken(Token::Type::Bang);
}

Token Lexer::readAmp() { // read '&' '&&' '&='
  tokenString = '&';
  if (nextChar() == '&') {
    appendAndNext();
    return makeToken(Token::Type::AmpAmp);
  }
  if (lastChar == '=') {
    appendAndNext();
    return makeToken(Token::Type::AmpEq);
  }
  return makeToken(Token::Type::Amp);
}

Token Lexer::readVBar() { // read '|' '||' '|='
  tokenString = '|';
  if (nextChar() == '|') {
    appendAndNext();
    return makeToken(Token::Type::VBarVBar);
  }
  if (lastChar == '=') {
    appendAndNext();
    return makeToken(Token::Type::VBarEq);
  }
  return makeToken(Token::Type::VBar);
}

// Token Lexer::readTilde() { error("not implemented");}

Token Lexer::readCaret() { // read '^' '^='
  tokenString = '^';
  if (nextChar() == '=') {
    appendAndNext();
    return makeToken(Token::Type::CaretEq);
  }
  return makeToken(Token::Type::Caret);
}
