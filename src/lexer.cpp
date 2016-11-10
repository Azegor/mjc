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

#include "util.hpp"

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

bool Lexer::tokenNameNeedsQuotes(Token::Type type) {
  switch (type) {
  case Token::Type::Eof:
  case Token::Type::Identifier:
  case Token::Type::IntLiteral:
  case Token::Type::ReservedKeyword:
    return false;
  default:
    return true;
  }
}

const char *Lexer::getTokenName(Token::Type type) {
  switch (type) {
  case Token::Type::Eof:
    return "End of File";

  case Token::Type::Identifier:
    return "identifier";
  case Token::Type::IntLiteral:
    return "integer literal";

  // -- keywords:
  case Token::Type::Boolean:
    return "boolean";
  case Token::Type::Class:
    return "class";
  case Token::Type::Else:
    return "else";
  case Token::Type::False:
    return "false";
  case Token::Type::If:
    return "if";
  case Token::Type::Int:
    return "int";
  case Token::Type::New:
    return "new";
  case Token::Type::Null:
    return "null";
  case Token::Type::Public:
    return "public";
  case Token::Type::Return:
    return "return";
  case Token::Type::Static:
    return "static";
  case Token::Type::This:
    return "this";
  case Token::Type::True:
    return "true";
  case Token::Type::Void:
    return "void";
  case Token::Type::While:
    return "while";

  // -- operators:
  case Token::Type::Amp:
    return "&";
  case Token::Type::AmpAmp:
    return "&&";
  case Token::Type::AmpEq:
    return "&=";
  case Token::Type::Bang:
    return "!";
  case Token::Type::BangEq:
    return "!=";
  case Token::Type::Caret:
    return "^";
  case Token::Type::CaretEq:
    return "^=";
  case Token::Type::Colon:
    return ":";
  case Token::Type::Comma:
    return ",";
  case Token::Type::Dot:
    return ".";
  case Token::Type::Eq:
    return "=";
  case Token::Type::EqEq:
    return "==";
  case Token::Type::Gt:
    return ">";
  case Token::Type::GtEq:
    return ">=";
  case Token::Type::GtGt:
    return ">>";
  case Token::Type::GtGtEq:
    return ">>=";
  case Token::Type::GtGtGt:
    return ">>>";
  case Token::Type::GtGtGtEq:
    return ">>>=";
  case Token::Type::LBrace:
    return "{";
  case Token::Type::LBracket:
    return "[";
  case Token::Type::LParen:
    return "(";
  case Token::Type::LtEq:
    return "<=";
  case Token::Type::LtLt:
    return "<<";
  case Token::Type::Lt:
    return "<";
  case Token::Type::LtLtEq:
    return "<<=";
  case Token::Type::Minus:
    return "-";
  case Token::Type::MinusEq:
    return "-=";
  case Token::Type::MinusMinus:
    return "--";
  case Token::Type::VBar:
    return "|";
  case Token::Type::VBarEq:
    return "|=";
  case Token::Type::VBarVBar:
    return "||";
  case Token::Type::Percent:
    return "%";
  case Token::Type::PercentEq:
    return "%=";
  case Token::Type::Plus:
    return "+";
  case Token::Type::PlusEq:
    return "+=";
  case Token::Type::PlusPlus:
    return "++";
  case Token::Type::QMark:
    return "?";
  case Token::Type::RBrace:
    return "}";
  case Token::Type::RBracket:
    return "]";
  case Token::Type::RParen:
    return ")";
  case Token::Type::Semicolon:
    return ";";
  case Token::Type::Slash:
    return "/";
  case Token::Type::SlashEq:
    return "/=";
  case Token::Type::Star:
    return "*";
  case Token::Type::StarEq:
    return "*=";
  case Token::Type::Tilde:
    return "~";

  case Token::Type::ReservedKeyword:
    return "reserved keyword"; // should never be used
  default:
    return "<unknown token type>";
  }
}

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

  if (lastChar == '\t') {
    ++column; // add extra space after tab, since they are printes as two spaces
  }

  if (!streamIsEof && nextCharPos == curBufferSize) {
    readIntoBuffer();
  }

  lastChar = static_cast<int>(buffer[nextCharPos++]);

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

    lineStartFileOffsets.push_back(bufferStartOffset + nextCharPos -
                                   static_cast<std::streamoff>(1));

    return lastChar;
  } // switch end

  ++column;
  return lastChar;
}

std::string Lexer::getCurrentLineFromInput(int lineNr) {
  int oldPos = input.tellg();
  input.clear(); // need to clear potential eof bit before seek
  input.seekg(lineStartFileOffsets.at(lineNr - 1));
  std::string res;
  int c = input.get();
  constexpr int maxLineLength = 1024;
  int lineLength = 0;
  while (c != '\r' && c != '\n' && input.good() &&
         ++lineLength < maxLineLength) {
    if (c == '\t') {
      res += "  "; // use 2 spaces for tabs
    } else {
      res.push_back(c);
    }
    c = input.get();
  }
  if (lineLength == maxLineLength) {
    res += "...";
  }
  input.seekg(oldPos);
  return res.empty() ? co::color_output(co::cyan, co::normal)("\\empty-line")
                     : res;
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

static bool isAlphaOrUnderscore(int c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool isAlphaNumOrUnderscore(int c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || c == '_';
}

static bool isDigit(int c) { return ('0' <= c && c <= '9'); }

Token Lexer::readNextToken() {

  if (skipCommentsAndWhitespaces()) {
    return readSlashFromSecondCharOn();
  }

  initToken(); // clear tokenstring, sets line/col to current pos

  // identifiers [_a-zA-Z][_a-zA-Z0-9]*
  if (isAlphaOrUnderscore(lastChar)) {
    tokenString = lastChar;
    while (isAlphaNumOrUnderscore(nextChar()))
      tokenString += lastChar;

    auto res = identifierTokens.find(tokenString);
    if (res != identifierTokens.end())
      return makeToken(res->second);
    return makeToken(Token::Type::Identifier);
  }

  // single 0 or leading 1-9:
  if (isDigit(lastChar)) {
    return readDecNumber();
  }

  //-----------
  // operators (single or multiple characters)
  switch (lastChar) {
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
  if (isEof()) {
    tokenString = "EOF";
    return makeToken(Token::Type::Eof);
  }

  // remaining single characters as tokens (i.e. operator symbols)
  tokenString = lastChar;
  Token::Type type = getSingleCharOpToken(lastChar);
  if (unlikely(type == Token::Type::none)) { // illegal character
    invalidCharError(lastChar);
  }
  nextChar(); // eat
  return makeToken(type);
}

/// returns true if char before last char was '/' (not from comment)
bool Lexer::skipCommentsAndWhitespaces() {
  while (true) {
  loophead:
    switch (lastChar) {
    case '/':
      initToken();             // in case we have a '/?' operator
      if (nextChar() == '*') { // multi line comment
        nextChar();
        while (likely(!isEof())) {
          if (unlikely(lastChar & 0b1000'0000)) {
            invalidCharError(lastChar);
          }
          if (lastChar == '*') {
            if (nextChar() == '/') {
              nextChar();    // eat '/'
              goto loophead; // continue while (true) loop
            } else {
              continue; // skip call to nextChar() for cases like '**/'
            }
          }
          nextChar();
        }
        error("unexpected end of file in multi line comment");
      } else {
        return true;
      }
      break;
    case ' ':
    case '\r':
    case '\n':
    case '\t':
      // skip all whitespaces
      do {
        nextChar();
      } while (isSpace(lastChar));
      break;
    default:
      return false;
    }
  }
}

Token Lexer::readSlashFromSecondCharOn() { // read '/' '/=' '/*'
  tokenString = '/';
  // character '/' was already next and lastChar points to the next char
  if (lastChar == '=') {
    appendAndNext();
    return makeToken(Token::Type::SlashEq);
  } else {
    return makeToken(Token::Type::Slash);
  }
}

Token Lexer::readDecNumber() { // read '0|[1-9][0-9]*'
  tokenString = lastChar;
  if ('0' == lastChar) {
    nextChar();
    return makeToken(Token::Type::IntLiteral);
  }
  while (isDigit(nextChar()))
    tokenString += lastChar;
  return makeToken(Token::Type::IntLiteral);
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
