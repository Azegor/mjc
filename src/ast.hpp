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

public:
  Node(SourceLocation loc) : location(std::move(loc)) {}
  virtual ~Node() {}
  virtual void accept(Visitor *visitor) { (void)visitor; }
};
using NodePtr = std::unique_ptr<Node>;

class Type : public Node {
public:
  virtual ~Type() = 0;
};
using TypePtr = std::unique_ptr<Type>;

class BlockStatement : public Node {
public:
  virtual ~BlockStatement() {}
};
using BlockStmtPtr = std::unique_ptr<BlockStatement>;

class Statement : public BlockStatement {
public:
  virtual ~Statement() {}
};
using StmtPtr = std::unique_ptr<Statement>;

class Expression : public Node {
public:
  virtual ~Expression() {}
};
using ExprPtr = std::unique_ptr<Expression>;

class Block : public Node {
  std::vector<BlockStmtPtr> statements;
  bool containsNothingExceptOneSingleLonelyEmtpyExpression;
};
using BlockPtr = std::unique_ptr<Block>;

class NonArrayType : public Type {
public:
  ~NonArrayType() {}
};

class PrimitiveType : public NonArrayType {
  enum class TypeType { Bool, Int, Void };
};

class ClassType : public NonArrayType {
  std::string name;
};

class ArrayType : public Type {
  NonArrayType elementType;
  int dimension;
};

class ExpressionStatement : public Statement {
  ExprPtr expr;
};

// IfStatement/WhileStatement/... oder nur If/While/...? ist ja schon im Ast namespace
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

class PrimaryExpression : public Expression {};

class NewArrayExpression : public PrimaryExpression {
  ArrayType type;
  ExprPtr size;
};

class NewObjectExpression : public PrimaryExpression {
  std::string name;
};

class IntLiteral : public PrimaryExpression {
  int32_t value;
};

class BoolLiteral : public PrimaryExpression {
  bool value;
};

class NullLiteral : public PrimaryExpression {};

class ThisLiteral : public PrimaryExpression {};

class Ident : public PrimaryExpression {
  std::string name;
};

class MethodInvocation : public Expression {
  ExprPtr left;
  std::string name;
  // might be empty
  std::vector<ExprPtr> arguments;
};

class FieldAccess : public Expression {
  // left.field_name
  ExprPtr left;
  std::string name;
};

class ArrayAccess : public Expression {
  ExprPtr expr;
  ExprPtr index;
};

class BinaryExpression : public Expression {
  ExprPtr left;
  ExprPtr right;
  enum class Op {
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
};

class UnaryExpression : public Expression {
protected:
  ExprPtr expression;
};

class NegExpression : public UnaryExpression {};

class NotExpression : public UnaryExpression {};

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
