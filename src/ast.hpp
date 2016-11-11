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

#include <iostream>
#include <memory>

#include "lexer.hpp"

namespace ast {

class Program;
class Block;
class BlockStatement;
class Class;
class Method;
class Field;
class PrimitiveType;
class ClassType;
class ArrayType;
using ClassPtr = std::unique_ptr<Class>;
class Visitor {
public:
  virtual ~Visitor() {}
  virtual void visitProgram(Program &program) { (void)program; }
  // virtual void visitBlock(Block &block) { (void)block; }
  // virtual void visitBlockStatement(BlockStatement &stmt) { (void)stmt; }
  virtual void visitClass(Class &klass) { (void)klass; }
  virtual void visitField(Field &field) { (void)field; }
  virtual void visitMethod(Method &method) { (void)method; }
  virtual void visitPrimitiveType(PrimitiveType &primitiveType) { (void)primitiveType; }
  virtual void visitClassType(ClassType &classType) { (void)classType; }
  virtual void visitArrayType(ArrayType &arrayType) { (void)arrayType; }
};

class Node {
protected:
  SourceLocation location;

  Node(SourceLocation loc) : location(std::move(loc)) {}

public:
  virtual void accept(Visitor *visitor) { (void)visitor; }
  virtual ~Node() = default;
  const SourceLocation &getLoc() const { return location; }
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

using BlockStmtList = std::vector<BlockStmtPtr>;
class Block : public Statement {
  BlockStmtList statements;
  bool containsNothingExceptOneSingleLonelyEmtpyExpression;

public:
  Block(SourceLocation loc, BlockStmtList statements, bool flag)
      : Statement(std::move(loc)), statements(std::move(statements)),
        containsNothingExceptOneSingleLonelyEmtpyExpression(flag) {}
};
using BlockPtr = std::unique_ptr<Block>;

class BasicType : public Type {
protected:
  BasicType(SourceLocation loc) : Type(std::move(loc)) {}
};
using BasicTypePtr = std::unique_ptr<BasicType>;

class PrimitiveType : public BasicType {
public:
  enum class TypeType { Boolean, Int, Void, None } type;
  PrimitiveType(SourceLocation loc, TypeType type)
      : BasicType(std::move(loc)), type(type) {}

  static TypeType getTypeForToken(Token::Type t) {
    switch (t) {
    case Token::Type::Boolean:
      return TypeType::Boolean;
    case Token::Type::Int:
      return TypeType::Int;
    case Token::Type::Void:
      return TypeType::Void;
    default:
      return TypeType::None;
    }
  }
  const TypeType &getType() { return type; }

  void accept(Visitor *visitor) override {
    visitor->visitPrimitiveType(*this);
  }
};

class ClassType : public BasicType {
  std::string name;

public:
  ClassType(SourceLocation loc, std::string name)
      : BasicType(std::move(loc)), name(std::move(name)) {}

  const std::string &getName() { return name; }

  void accept(Visitor *visitor) override {
    visitor->visitClassType(*this);
  }
};

class ArrayType : public Type {
  BasicTypePtr elementType;
  int dimension;

public:
  ArrayType(SourceLocation loc, BasicTypePtr elementType, int dimension)
      : Type(std::move(loc)), elementType(std::move(elementType)),
        dimension(dimension) {}
  BasicType &getElementType() { return *elementType; }
  const int getDimension() { return dimension; }

  void accept(Visitor *visitor) override {
    visitor->visitArrayType(*this);
  }
};
using ArrayTypePtr = std::unique_ptr<ArrayType>;

class ExpressionStatement : public Statement {
  ExprPtr expr;

public:
  ExpressionStatement(SourceLocation loc, ExprPtr expr)
      : Statement(std::move(loc)), expr(std::move(expr)) {}
};

class IfStatement : public Statement {
  ExprPtr condition;
  StmtPtr thenStmt;
  StmtPtr elseStmt;

public:
  IfStatement(SourceLocation loc, ExprPtr condition, StmtPtr thenStmt,
              StmtPtr elseStmt)
      : Statement(std::move(loc)), condition(std::move(condition)),
        thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}
};

class WhileStatement : public Statement {
  ExprPtr condition;
  StmtPtr statement;

public:
  WhileStatement(SourceLocation loc, ExprPtr condition, StmtPtr statement)
      : Statement(std::move(loc)), condition(std::move(condition)),
        statement(std::move(statement)) {}
};

class ReturnStatement : public Statement {
  // might be nullptr
  ExprPtr expr;

public:
  ReturnStatement(SourceLocation loc, ExprPtr expr)
      : Statement(std::move(loc)), expr(std::move(expr)) {}
};

class Field : public Node {
  TypePtr type;
  std::string name;

public:
  Field(SourceLocation loc, TypePtr type, std::string name)
      : Node(std::move(loc)), type(std::move(type)), name(std::move(name)) {}

  const std::string &getName() { return name; }
  Type &getType() { return *type; }

  bool operator< (const Field &other) {
    return name < other.name;
  }

};
using FieldPtr = std::unique_ptr<Field>;

class Parameter : public Node {
  TypePtr type;
  std::string name;

public:
  Parameter(SourceLocation loc, TypePtr type, std::string name)
      : Node(std::move(loc)), type(std::move(type)), name(std::move(name)) {}
};
using ParameterPtr = std::unique_ptr<Parameter>;
using ParameterList = std::vector<ParameterPtr>;

class Method : public Node {
  TypePtr returnType;
  std::string name;
  // might be empty
  ParameterList parameters;
  BlockPtr block;

public:
  Method(SourceLocation loc, TypePtr returnType, std::string name,
         ParameterList parameters, BlockPtr block)
      : Node(std::move(loc)), returnType(std::move(returnType)),
        name(std::move(name)), parameters(std::move(parameters)),
        block(std::move(block)) {}

  const std::string &getName() { return name; }
  Type &getReturnType() { return *returnType; }
  Block &getBlock() { return *block; }

  ParameterList getParameters() {
    ParameterList result;
    for(ParameterList::size_type i=0; i<parameters.size(); i++) {
      result.push_back(std::move(parameters[i]));
    }
    return result;
  }

  bool operator< (Method &other) {
    return name < other.name;
  }
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

  void accept(Visitor *visitor) override {
    std::sort(methods.begin(), methods.end());
    for (auto &mp : methods) {
      visitor->visitMethod(*mp);
    }

    std::sort(fields.begin(), fields.end());
    for (auto &fp : fields) {
      visitor->visitField(*fp);
    }
  }

  const std::string &getName() { return name; }
};

class Program : public Node {
  std::vector<ClassPtr> classes;

public:
  Program(SourceLocation loc, std::vector<ClassPtr> classes)
      : Node(loc), classes(std::move(classes)) {}

  void accept(Visitor *visitor) override {
    for (auto &cp : classes) {
      visitor->visitClass(*cp);
    }
    visitor->visitProgram(*this);
  }
};
using ProgramPtr = std::unique_ptr<Program>;

class VariableDeclaration : public BlockStatement {
  TypePtr type;
  std::string name;
  // might be nullptr
  ExprPtr initializer;

public:
  VariableDeclaration(SourceLocation loc, TypePtr type, std::string name,
                      ExprPtr initializer)
      : BlockStatement(std::move(loc)), type(std::move(type)),
        name(std::move(name)), initializer(std::move(initializer)) {}
};

class PrimaryExpression : public Expression {
protected:
  PrimaryExpression(SourceLocation loc) : Expression(std::move(loc)) {}
};

class NewArrayExpression : public PrimaryExpression {
  ArrayTypePtr arrayType;
  ExprPtr size;

public:
  NewArrayExpression(SourceLocation loc, ArrayTypePtr arrayType, ExprPtr size)
      : PrimaryExpression(std::move(loc)), arrayType(std::move(arrayType)),
        size(std::move(size)) {}
};

class NewObjectExpression : public PrimaryExpression {
  std::string name;

public:
  NewObjectExpression(SourceLocation loc, std::string name)
      : PrimaryExpression(std::move(loc)), name(std::move(name)) {}
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
  Ident(SourceLocation loc, std::string name)
      : PrimaryExpression(std::move(loc)), name(std::move(name)) {}
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
  ArrayAccess(SourceLocation loc, ExprPtr lhs, ExprPtr index)
      : Expression(std::move(loc)), array(std::move(lhs)),
        index(std::move(index)) {}
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
  ExprPtr expression;
  enum class Op {
    Not,
    Neg,
    None

  } operation;

public:
  UnaryExpression(SourceLocation loc, ExprPtr expression, Op operation)
      : Expression(std::move(loc)), expression(std::move(expression)),
        operation(operation) {}

  static Op getOpForToken(Token::Type t) {
    switch (t) {
    case Token::Type::Minus:
      return Op::Neg;
    case Token::Type::Bang:
      return Op::Not;
    default:
      return Op::None;
    }
  }
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

template <typename T, typename... Args>
std::unique_ptr<T> make_Ptr(SourceLocation loc, Args &&... args) {
  return std::unique_ptr<T>{new T(loc, std::forward<Args>(args)...)};
}

} // namespace ast

#endif // AST_H
