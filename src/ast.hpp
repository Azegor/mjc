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

#ifndef AST_H
#define AST_H

#include <memory>

#include "lexer.hpp"

namespace ast {

class Visitor {
  // Foobar
};

class Node {
protected:
  SourceLocation location;

  Node(SourceLocation loc) : location(std::move(loc)) {}

public:
  virtual void accept(Visitor *visitor) { (void)visitor; }
  virtual ~Node() = default;
};
using NodePtr = std::unique_ptr<Node>;

class Type : public Node {
protected:
  Type(SourceLocation loc) : Node(std::move(loc)) {}
};
using TypePtr = std::unique_ptr<Type>;

class BlockStatement : public Node {
protected:
  BlockStatement(SourceLocation loc) : Node(std::move(loc)) {}
};
using BlockStmtPtr = std::unique_ptr<BlockStatement>;

class Statement : public BlockStatement {
protected:
  Statement(SourceLocation loc) : BlockStatement(std::move(loc)) {}
};
using StmtPtr = std::unique_ptr<Statement>;

class Expression : public Node {
protected:
  Expression(SourceLocation loc) : Node(std::move(loc)) {}
};
using ExprPtr = std::unique_ptr<Expression>;
using ExprList = std::vector<ExprPtr>;

class Block : public Statement {
  std::vector<BlockStmtPtr> statements;
  bool containsNothingExceptOneSingleLonelyEmtpyExpression;

public:
  Block(SourceLocation loc) : Statement(std::move(loc)) {}
};
using BlockPtr = std::unique_ptr<Block>;

class NonArrayType : public Type {
protected:
  NonArrayType(SourceLocation loc) : Type(std::move(loc)) {}
};
using NonArrayTypePtr = std::unique_ptr<NonArrayType>;

class PrimitiveType : public NonArrayType {
  enum class TypeType { Bool, Int, Void } type;

public:
  PrimitiveType(SourceLocation loc) : NonArrayType(std::move(loc)) {}
};

class ClassType : public NonArrayType {
  std::string name;

public:
  ClassType(SourceLocation loc) : NonArrayType(std::move(loc)) {}
};

class ArrayType : public Type {
  NonArrayTypePtr elementType;
  int dimension;

public:
  ArrayType(SourceLocation loc, NonArrayTypePtr elementType)
      : Type(std::move(loc)), elementType(std::move(elementType)) {}
};

class ExpressionStatement : public Statement {
  ExprPtr expr;
};

// IfStatement/WhileStatement/... oder nur If/While/...? ist ja schon im Ast
// namespace
class IfStatement : public Statement {
  ExprPtr condition;
  StmtPtr then_statement;
  StmtPtr else_statement;
};

class WhileStatement : public Statement {
  ExprPtr condition;
  StmtPtr statement;
};

class ReturnStatement : public Statement {
  // might be nullptr
  ExprPtr expr;
};

class Field : public Node {
  std::string name;
  TypePtr type;
};
using FieldPtr = std::unique_ptr<Field>;

class Parameter : public Node {
  std::string name;
  TypePtr type;
};
using ParameterPtr = std::unique_ptr<Parameter>;
using ParameterList = std::vector<ParameterPtr>;

class Method : public Node {
  std::string name;
  TypePtr returnType;
  // might be empty
  std::vector<Parameter> parameters;
  BlockPtr block;
};
using MethodPtr = std::unique_ptr<Method>;

class MainMethod : public Node {
  std::string name;
  std::string argName;
  BlockPtr block;

public:
  MainMethod(SourceLocation loc, std::string name, std::string argName,
             BlockPtr block)
      : Node(std::move(loc)), name(std::move(name)),
        argName(std::move(argName)), block(std::move(block)) {}
};
using MainMethodPtr = std::unique_ptr<MainMethod>;

class Class : public Node {
  std::string name;
  std::vector<FieldPtr> fields;
  std::vector<MethodPtr> methods;
  std::vector<MainMethodPtr> mainMethods;

public:
  Class(SourceLocation loc, std::string name, std::vector<FieldPtr> fields,
        std::vector<MethodPtr> methods, std::vector<MainMethodPtr> mainMethods)
      : Node(loc), name(std::move(name)), fields(std::move(fields)),
        methods(std::move(methods)), mainMethods(std::move(mainMethods)) {}
};
using ClassPtr = std::unique_ptr<Class>;

class Program : public Node {
  std::vector<ClassPtr> classes;

public:
  Program(SourceLocation loc, std::vector<ClassPtr> classes)
      : Node(loc), classes(std::move(classes)) {}
};
using ProgramPtr = std::unique_ptr<Program>;

class VariableDeclaration : public BlockStatement {
  TypePtr type;
  std::string name;
  // might be nullptr
  ExprPtr initializer;
};

class PrimaryExpression : public Expression {
protected:
  PrimaryExpression(SourceLocation loc) : Expression(std::move(loc)) {}
};

class NewArrayExpression : public PrimaryExpression {
  ArrayType type;
  ExprPtr size;

public:
  NewArrayExpression(SourceLocation loc, ArrayType arrayType)
      : PrimaryExpression(std::move(loc)), type(std::move(arrayType)) {}
};

class NewObjectExpression : public PrimaryExpression {
  std::string name;

public:
  NewObjectExpression(SourceLocation loc) : PrimaryExpression(std::move(loc)) {}
};

class IntLiteral : public PrimaryExpression {
  int32_t value;

public:
  IntLiteral(SourceLocation loc) : PrimaryExpression(std::move(loc)) {}
};

class BoolLiteral : public PrimaryExpression {
  bool value;

public:
  BoolLiteral(SourceLocation loc, bool val)
      : PrimaryExpression(std::move(loc)), value(val) {}
};

class NullLiteral : public PrimaryExpression {
public:
  NullLiteral(SourceLocation loc) : PrimaryExpression(std::move(loc)) {}
};

class ThisLiteral : public PrimaryExpression {
public:
  ThisLiteral(SourceLocation loc) : PrimaryExpression(std::move(loc)) {}
};

class Ident : public PrimaryExpression {
  std::string name;

public:
  Ident(SourceLocation loc) : PrimaryExpression(std::move(loc)) {}
};

class MethodInvocation : public Expression {
  ExprPtr left; // set to this if call from within function
  std::string name;
  // might be empty
  std::vector<ExprPtr> arguments;

public:
  MethodInvocation(SourceLocation loc, ExprPtr lhs, std::string methodName,
                   ExprList methodArgs)
      : Expression(std::move(loc)), left(std::move(lhs)),
        name(std::move(methodName)), arguments(std::move(methodArgs)) {}
};

class FieldAccess : public Expression {
  // left.field_name
  ExprPtr left; // set to this if call from within function
  std::string name;

public:
  FieldAccess(SourceLocation loc, ExprPtr lhs, std::string memberName)
      : Expression(std::move(loc)), left(std::move(lhs)),
        name(std::move(memberName)) {}
};

class ArrayAccess : public Expression {
  ExprPtr array;
  ExprPtr index;

public:
  ArrayAccess(SourceLocation loc, ExprPtr lhs, ExprPtr index) : Expression(std::move(loc)), array(std::move(lhs)), index(std::move(index)) {}
};

class BinaryExpression : public Expression {
  ExprPtr left;
  ExprPtr right;
  enum class Op {
    None,
    Assign,
    Or,
    And,
    Equals,
    NotEquals,
    Less,
    LessEquals,
    Greater,
    GreaterEquals,
    Plus,
    Minus,
    Mul,
    Div,
    Mod
  } operation;

public:
  BinaryExpression(SourceLocation loc, ExprPtr lhs, ExprPtr rhs, Op op)
      : Expression(std::move(loc)), left(std::move(lhs)), right(std::move(rhs)),
        operation(op) {}

  static Op getOpForToken(Token::Type t) {
    switch (t) {
    case Token::Type::Eq:
      return Op::Assign;
    case Token::Type::VBarVBar:
      return Op::Or;
    case Token::Type::AmpAmp:
      return Op::And;
    case Token::Type::EqEq:
      return Op::Equals;
    case Token::Type::BangEq:
      return Op::NotEquals;
    case Token::Type::Lt:
      return Op::Less;
    case Token::Type::LtEq:
      return Op::LessEquals;
    case Token::Type::Gt:
      return Op::Greater;
    case Token::Type::GtEq:
      return Op::GreaterEquals;
    case Token::Type::Plus:
      return Op::Plus;
    case Token::Type::Minus:
      return Op::Minus;
    case Token::Type::Star:
      return Op::Mul;
    case Token::Type::Slash:
      return Op::Div;
    case Token::Type::Percent:
      return Op::Mod;
    default:
      return Op::None;
    }
  }
};

class UnaryExpression : public Expression {
protected:
  ExprPtr expression;

  UnaryExpression(SourceLocation loc) : Expression(std::move(loc)) {}
};

class NegExpression : public UnaryExpression {
public:
  NegExpression(SourceLocation loc) : UnaryExpression(std::move(loc)) {}
};

class NotExpression : public UnaryExpression {
public:
  NotExpression(SourceLocation loc) : UnaryExpression(std::move(loc)) {}
};

template <typename St, typename... Args>
StmtPtr make_SPtr(SourceLocation loc, Args &&... args) {
  return StmtPtr{new St(loc, std::forward<Args>(args)...)};
}
// for copy/move constructor
template <typename St> StmtPtr make_SPtr(const St &s) {
  return StmtPtr{new St(s)};
}

template <typename Ex, typename... Args>
ExprPtr make_EPtr(SourceLocation loc, Args &&... args) {
  return ExprPtr{new Ex(loc, std::forward<Args>(args)...)};
}
// for copy/move constructor
template <typename Ex> ExprPtr make_EPtr(const Ex &e) {
  return ExprPtr{new Ex(e)};
}
}

#endif // AST_H
