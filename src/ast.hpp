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
  SourceLocation location;

  Node(SourceLocation loc) : location(loc) {}
  virtual ~Node() {}

  virtual void accept(Visitor *visitor) = 0;
};

class Type : public Node {
};

class BlockStatement : public Node {
};

class Statement : public BlockStatement {
};

class Expression : public Node {
};

class Block : public Node {
  std::vector<BlockStatement> statements;
  bool containsNothingExceptOneSingleLonelyEmtpyExpression;
};

using NodePtr = std::unique_ptr<Node>;
using TypePtr = std::unique_ptr<Type>;
using StmtPtr = std::unique_ptr<Stmt>;
using ExprPtr = std::unique_ptr<Expr>;
using BlockPtr = std::unique_ptr<Block>;

⁠template <typename St, typename... Args>
StmtPtr make_SPtr(SourceLocation loc, Args &&... args) {
  return StmtPtr{new St(loc, std::forward<Args>(args)...)};
};
// for copy/move constructor
template <typename St> StmtPtr make_SPtr(const St &s) {
  return StmtPtr{new St(s)};
};

template <typename Ex, typename... Args>
ExprPtr make_EPtr(SourceLocation loc, Args &&... args) {
  return ExprPtr{new Ex(loc, std::forward<Args>(args)...)};
};
// for copy/move constructor
template <typename Ex> ExprPtr make_EPtr(const Ex &e) {
  return ExprPtr{new Ex(e)};
};

class NonArrayType : public Type {
};

class PrimitiveType : public NonArrayType {
  enum class {
    Bool,
    Int,
    Void
  };
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

class MainMethod : public Node {
  std::string name;
  std::string argName;
  BlockPtr block;
};

class Class : public Node {
  std::string name;
  std::vector<Field> fields;
  std::vector<Method> methods;
  std::vector<MainMethod> mainmethods;
};

class Program : public Node {
  std::vector<Class> classes;
};

class VariableDeclaration : public Expression {
  TypePtr type;
  std::string name;
  // might be nullptr
  ExprPtr initializer;
};

class PrimaryExpression : public Expression {
};

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

class NullLiteral : public PrimaryExpression {
  // ???
};

class ThisLiteral : public PrimaryExpression {
};

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
  ExptrPtr index;
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
  };
};

class UnaryExpression : public Expression {
  ExprPtr expression;
};

class NegExpression : public UnaryExpression {
};

class NotExpression : public UnaryExpression {
};



};


#endif // AST_H
