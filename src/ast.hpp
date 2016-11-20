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

#include <algorithm>
#include <iostream>
#include <memory>

#include "lexer.hpp"
#include "symboltable.hpp"

namespace sem {
enum class TypeKind {
  Array,
  Class,
  Bool,
  Int,
  Void,
  Null, // Umm?
  Unresolved
};

struct Type {
  TypeKind kind;
  // For arrays, e.g. int[] -> innerKind=int
  TypeKind innerKind = TypeKind::Unresolved;
  // Both class types and arrays need a name
  std::string name;
  int dimension = 0;
  Type() { kind = TypeKind::Unresolved; }

  void setInt() { kind = TypeKind::Int; }
  void setBool() { kind = TypeKind::Bool; }
  void setArray(TypeKind innerKind, int dimension, std::string name = "") {
    assert(innerKind != TypeKind::Array);
    this->kind = TypeKind::Array;
    this->name = name;
    this->innerKind = innerKind;
    this->dimension = dimension;
  }
  void setClass(std::string name) {
    this->kind = TypeKind::Class;
    this->name = name;
  }
  void setNull() { kind = TypeKind::Null; }
  void setVoid() { kind = TypeKind::Void; }

  bool isInt() const { return kind == TypeKind::Int; }
  bool isBool() const { return kind == TypeKind::Bool; }
  bool isClass() const { return kind == TypeKind::Class; }
  bool isArray() const { return kind == TypeKind::Array; }
  bool isNull() const { return kind == TypeKind::Null; }
  bool isVoid() const { return kind == TypeKind::Void; }

  bool operator==(const sem::Type &other) const {
    switch (this->kind) {
    case TypeKind::Class:
      return other.kind == TypeKind::Class && this->name == other.name;
    case TypeKind::Array:
      if (other.kind != TypeKind::Array || this->dimension != other.dimension ||
          this->innerKind != other.innerKind) {
        return false;
      }
      if (this->innerKind == TypeKind::Class) {
        return this->name == other.name;
      } else {
        return true;
      }
    default:
      return this->kind == other.kind;
    }
  }
  bool operator>=(const sem::Type &other) const {
    switch (this->kind) {
    case TypeKind::Class:
    case TypeKind::Array:
      if (other.kind == TypeKind::Null) {
        return true;
      }
    default:
      return *this == other;
    }
  }
  bool operator!=(const sem::Type &other) const { return !(*this == other); }
};

enum class ControlFlowBehavior {
  MayContinue, // code flow continues after the statement
  Return,      // returns the function
};

inline ControlFlowBehavior combineCFB(ControlFlowBehavior c1,
                                      ControlFlowBehavior c2) {
  if (c1 == c2) {
    return c1;
  }
  return ControlFlowBehavior::MayContinue;
}

} // namespace sem
std::ostream &operator<<(std::ostream &o, const sem::Type &t);

namespace ast {
struct SortUniquePtrPred {
  template <typename T>
  bool operator()(const std::unique_ptr<T> &lhs,
                  const std::unique_ptr<T> &rhs) const {
    return *lhs < *rhs;
  }
};
struct UniquePtrEqPred {
  template <typename T>
  bool operator()(const std::unique_ptr<T> &lhs,
                  const std::unique_ptr<T> &rhs) const {
    return *lhs == *rhs;
  }
};

class Visitor;

class Node {
protected:
  SourceLocation location;

  Node(SourceLocation loc) : location(std::move(loc)) {}

public:
  virtual void accept(Visitor *) {}
  virtual void acceptChildren(Visitor *) {}
  virtual ~Node() = default;
  const SourceLocation &getLoc() const { return location; }
};
using NodePtr = std::unique_ptr<Node>;

class Program;
class Block;
class BlockStatement;
class Class;
class FieldList;
class MethodList;
class MainMethodList;
class Field;
class RegularMethod;
class MainMethod;
class Parameter;
class PrimitiveType;
class ClassType;
class ArrayType;
class VariableDeclaration;
class ExpressionStatement;
class IfStatement;
class WhileStatement;
class ReturnStatement;
class NewArrayExpression;
class NewObjectExpression;
class IntLiteral;
class BoolLiteral;
class NullLiteral;
class ThisLiteral;
class VarRef;
class MethodInvocation;
class FieldAccess;
class ArrayAccess;
class BinaryExpression;
class UnaryExpression;
using ClassPtr = std::unique_ptr<Class>;

class SemanticError : public CompilerError {
public:
  const SourceLocation srcLoc;
  const std::string filename, message, errorLine;
  const bool reportAtScopeEnd;
  SemanticError(SourceLocation srcLoc, std::string filename,
                std::string message, std::string errorLine,
                bool reportAtScopeEnd)
      : srcLoc(std::move(srcLoc)), filename(std::move(filename)),
        message(std::move(message)), errorLine(std::move(errorLine)),
        reportAtScopeEnd(reportAtScopeEnd) {}
  const char *what() const noexcept override { return message.c_str(); }

  virtual void writeErrorMessage(std::ostream &out) const override {
    co::color_ostream<std::ostream> cl_out(out);
    auto reportToken = reportAtScopeEnd ? srcLoc.endToken : srcLoc.startToken;
    cl_out << co::mode(co::bold) << filename << ':' << reportToken.line << ':'
           << reportToken.col << ": " << co::color(co::red)
           << "error: " << co::reset << message << std::endl;
    srcLoc.writeErrorLineHighlight(out, errorLine);
  }
};

class Visitor {
public:
  virtual ~Visitor() {}
  virtual void visitProgram(Program &program);
  virtual void visitClass(Class &klass);
  virtual void visitFieldList(FieldList &fieldList);
  virtual void visitMethodList(MethodList &methodList);
  virtual void visitMainMethodList(MainMethodList &mainMethodList);
  virtual void visitField(Field &field);
  virtual void visitRegularMethod(RegularMethod &method);
  virtual void visitMainMethod(MainMethod &mainMethod);
  virtual void visitParameter(Parameter &parameter);
  virtual void visitPrimitiveType(PrimitiveType &primitiveType);
  virtual void visitClassType(ClassType &classType);
  virtual void visitArrayType(ArrayType &arrayType);
  virtual void visitBlock(Block &block);
  virtual void
  visitVariableDeclaration(VariableDeclaration &variableDeclartion);
  virtual void visitExpressionStatement(ExpressionStatement &exprStmt);
  virtual void visitIfStatement(IfStatement &ifStatement);
  virtual void visitWhileStatement(WhileStatement &whileStatement);
  virtual void visitReturnStatement(ReturnStatement &returnStatement);
  virtual void visitNewArrayExpression(NewArrayExpression &newArrayExpression);
  virtual void
  visitNewObjectExpression(NewObjectExpression &newObjectExpression);
  virtual void visitIntLiteral(IntLiteral &intLiteral);
  virtual void visitBoolLiteral(BoolLiteral &boolLiteral);
  virtual void visitNullLiteral(NullLiteral &nullLiteral);
  virtual void visitThisLiteral(ThisLiteral &thisLiteral);
  virtual void visitVarRef(VarRef &varRef);
  virtual void visitMethodInvocation(MethodInvocation &methodInvocation);
  virtual void visitFieldAccess(FieldAccess &fieldAccess);
  virtual void visitArrayAccess(ArrayAccess &arrayAccess);
  virtual void visitBinaryExpression(BinaryExpression &binaryExpression);
  virtual void visitUnaryExpression(UnaryExpression &unaryExpression);
};

class Type : public Node {
protected:
  Type(SourceLocation loc) : Node(std::move(loc)) {}

public:
  virtual sem::Type getSemaType() const = 0;
};
using TypePtr = std::unique_ptr<Type>;

class BlockStatement : public Node {
protected:
  BlockStatement(SourceLocation loc) : Node(std::move(loc)) {}

public:
  sem::ControlFlowBehavior cfb = sem::ControlFlowBehavior::MayContinue;
};
using BlockStmtPtr = std::unique_ptr<BlockStatement>;

class Statement : public BlockStatement {
protected:
  Statement(SourceLocation loc) : BlockStatement(std::move(loc)) {}
};
using StmtPtr = std::unique_ptr<Statement>;

class Expression : public Node {
public:
  sem::Type targetType;

protected:
  Expression(SourceLocation loc) : Node(std::move(loc)) {}
};

class RValueExpression : public Expression {
public:
  RValueExpression(SourceLocation loc) : Expression(loc) {}
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

  std::vector<BlockStatement *> getStatements() {
    std::vector<BlockStatement *> result;
    for (BlockStmtList::size_type i = 0; i < statements.size(); i++) {
      result.push_back(statements[i].get());
    }
    return result;
  }

  bool getContainsNothingExceptOneSingleLonelyEmptyExpression() const {
    return containsNothingExceptOneSingleLonelyEmtpyExpression;
  }

  void accept(Visitor *visitor) override { visitor->visitBlock(*this); }
  void acceptChildren(Visitor *visitor) override {
    for (auto &stmt : statements) {
      if (stmt != nullptr)
        stmt->accept(visitor);
    }
  }
};
using BlockPtr = std::unique_ptr<Block>;

class BasicType : public Type {
protected:
  BasicType(SourceLocation loc) : Type(std::move(loc)) {}
};
using BasicTypePtr = std::unique_ptr<BasicType>;

class PrimitiveType : public BasicType {
public:
  enum class PrimType { Boolean, Int, Void, None };

private:
  PrimType type;

public:
  PrimitiveType(SourceLocation loc, PrimType type)
      : BasicType(std::move(loc)), type(type) {}

  static PrimType getTypeForToken(Token::Type t) {
    switch (t) {
    case Token::Type::Boolean:
      return PrimType::Boolean;
    case Token::Type::Int:
      return PrimType::Int;
    case Token::Type::Void:
      return PrimType::Void;
    default:
      return PrimType::None;
    }
  }

  const PrimType &getPrimType() const { return type; }

  sem::Type getSemaType() const override {
    sem::Type res;
    switch (type) {
    case PrimType::Boolean:
      res.setBool();
      break;
    case PrimType::Int:
      res.setInt();
      break;
    case PrimType::Void:
      res.setVoid();
      break;
    default:
      break;
    }
    return res;
  }

  void accept(Visitor *visitor) override { visitor->visitPrimitiveType(*this); }
};

class ClassType : public BasicType {
  std::string name;
  Class *classDef = nullptr;

public:
  ClassType(SourceLocation loc, std::string name)
      : BasicType(std::move(loc)), name(std::move(name)) {}

  const std::string &getName() const { return name; }

  void accept(Visitor *visitor) override { visitor->visitClassType(*this); }
  void setDef(Class *def) { classDef = def; }
  Class *getDef() const { return classDef; }

  sem::Type getSemaType() const override {
    sem::Type res;
    res.setClass(getName());
    return res;
  }
};

class ArrayType : public Type {
  BasicTypePtr elementType;
  int dimension;

public:
  ArrayType(SourceLocation loc, BasicTypePtr elementType, int dimension)
      : Type(std::move(loc)), elementType(std::move(elementType)),
        dimension(dimension) {}
  BasicType *getElementType() const { return elementType.get(); }
  int getDimension() const { return dimension; }

  void accept(Visitor *visitor) override { visitor->visitArrayType(*this); }
  void acceptChildren(Visitor *visitor) override {
    elementType->accept(visitor);
  }

  sem::Type getSemaType() const override {
    sem::Type res;
    auto innerType = elementType->getSemaType();
    res.setArray(innerType.kind, dimension, innerType.name);
    return res;
  }
};
using ArrayTypePtr = std::unique_ptr<ArrayType>;

class ExpressionStatement : public Statement {
  ExprPtr expr;

public:
  ExpressionStatement(SourceLocation loc, ExprPtr expr)
      : Statement(std::move(loc)), expr(std::move(expr)) {}

  Expression *getExpression() const { return expr.get(); }

  void accept(Visitor *visitor) override {
    visitor->visitExpressionStatement(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    this->expr->accept(visitor);
  }
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

  Expression *getCondition() const { return condition.get(); }
  Statement *getThenStatement() const { return thenStmt.get(); }
  Statement *getElseStatement() const { return elseStmt.get(); }

  void accept(Visitor *visitor) override { visitor->visitIfStatement(*this); }
  void acceptChildren(Visitor *visitor) override {
    condition->accept(visitor);
    if (thenStmt != nullptr)
      thenStmt->accept(visitor);

    if (elseStmt != nullptr)
      elseStmt->accept(visitor);
  }
};

class WhileStatement : public Statement {
  ExprPtr condition;
  StmtPtr statement;

public:
  WhileStatement(SourceLocation loc, ExprPtr condition, StmtPtr statement)
      : Statement(std::move(loc)), condition(std::move(condition)),
        statement(std::move(statement)) {}

  Expression *getCondition() const { return condition.get(); }
  Statement *getStatement() const { return statement.get(); }

  void accept(Visitor *visitor) override {
    visitor->visitWhileStatement(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    condition->accept(visitor);
    if (statement != nullptr)
      statement->accept(visitor);
  }
};

class ReturnStatement : public Statement {
  // might be nullptr
  ExprPtr expr;

public:
  ReturnStatement(SourceLocation loc, ExprPtr expr)
      : Statement(std::move(loc)), expr(std::move(expr)) {}
  Expression *getExpression() const { return expr.get(); }

  void accept(Visitor *visitor) override {
    visitor->visitReturnStatement(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    if (expr != nullptr)
      expr->accept(visitor);
  }
};

class Field : public Node, public SymbolTable::Definition {
  TypePtr type;
  SymbolTable::Symbol &symbol;

public:
  Field(SourceLocation loc, TypePtr type, SymbolTable::Symbol &sym)
      : Node(std::move(loc)), type(std::move(type)), symbol(sym) {}

  const std::string &getName() const { return symbol.name; }
  SymbolTable::Symbol &getSymbol() const override { return symbol; }
  Type *getType() const override { return type.get(); }

  void accept(Visitor *visitor) override { visitor->visitField(*this); }

  void acceptChildren(Visitor *visitor) override { type->accept(visitor); }

  bool operator<(const Field &o) const { return symbol.name < o.symbol.name; }
  bool operator==(const Field &o) const { return symbol.name == o.symbol.name; }
};
using FieldPtr = std::unique_ptr<Field>;

class Parameter : public Node, public SymbolTable::Definition {
  TypePtr type;
  SymbolTable::Symbol &symbol;

public:
  Parameter(SourceLocation loc, TypePtr type, SymbolTable::Symbol &sym)
      : Node(std::move(loc)), type(std::move(type)), symbol(sym) {}

  void accept(Visitor *visitor) override { visitor->visitParameter(*this); }
  void acceptChildren(Visitor *visitor) override { type->accept(visitor); }
  const std::string &getName() const { return symbol.name; }
  SymbolTable::Symbol &getSymbol() const override { return symbol; }
  Type *getType() const override { return type.get(); }
};
using ParameterPtr = std::unique_ptr<Parameter>;
using ParameterList = std::vector<ParameterPtr>;

class Method : public Node {
public:
  Method(SourceLocation loc) : Node(loc) {}
  virtual Type *getReturnType() const = 0;
  virtual const std::string &getName() const = 0;

  bool operator<(const Method &o) const { return getName() < o.getName(); }
};

class RegularMethod : public Method {
  TypePtr returnType;
  std::string name;
  // might be empty
  ParameterList parameters;
  BlockPtr block;

public:
  RegularMethod(SourceLocation loc, TypePtr returnType, std::string name,
                ParameterList parameters, BlockPtr block)
      : Method(std::move(loc)), returnType(std::move(returnType)),
        name(std::move(name)), parameters(std::move(parameters)),
        block(std::move(block)) {}

  const std::string &getName() const override { return name; }
  Type *getReturnType() const override { return returnType.get(); }
  Block *getBlock() const { return block.get(); }

  std::vector<Parameter *> getParameters() {
    std::vector<Parameter *> result;
    for (ParameterList::size_type i = 0; i < parameters.size(); i++) {
      result.push_back(parameters[i].get());
    }
    return result;
  }

  bool operator==(const RegularMethod &other) const {
    return name == other.name;
  }

  void accept(Visitor *visitor) override { visitor->visitRegularMethod(*this); }

  void acceptChildren(Visitor *visitor) override {
    returnType->accept(visitor);
    for (auto &param : parameters) {
      param->accept(visitor);
    }
    block->accept(visitor);
  }
};
using RegularMethodPtr = std::unique_ptr<RegularMethod>;

class MainMethod : public Method {
  std::string name;
  SymbolTable::Symbol &argSymbol;
  BlockPtr block;

public:
  MainMethod(SourceLocation loc, std::string name, SymbolTable::Symbol &argName,
             BlockPtr block)
      : Method(std::move(loc)), name(std::move(name)), argSymbol(argName),
        block(std::move(block)) {}

  void accept(Visitor *visitor) override { visitor->visitMainMethod(*this); }
  void acceptChildren(Visitor *visitor) override { block->accept(visitor); }

  const std::string &getName() const override { return name; }
  Type *getReturnType() const override {
    static PrimitiveType type({}, PrimitiveType::PrimType::Void);
    return &type;
  }
  const std::string &getArgName() const { return argSymbol.name; }
  SymbolTable::Symbol &getArgSymbol() const { return argSymbol; }
  Block *getBlock() const { return block.get(); }
  bool operator==(const MainMethod &o) const { return name == o.name; }
};
using MainMethodPtr = std::unique_ptr<MainMethod>;

class FieldList : public Node {
public:
  std::vector<FieldPtr> fields;

  FieldList(std::vector<FieldPtr> fields)
      : Node({}), fields(std::move(fields)) {}
  void accept(Visitor *visitor) override { visitor->visitFieldList(*this); }
  void acceptChildren(Visitor *visitor) override {
    std::sort(fields.begin(), fields.end(), SortUniquePtrPred());
    for (auto &e : fields) {
      visitor->visitField(*e);
    }
  }

  void sortFields() {
    std::stable_sort(fields.begin(), fields.end(), ast::SortUniquePtrPred());
  }
};

class MethodList : public Node {
public:
  std::vector<RegularMethodPtr> methods;

  MethodList(std::vector<RegularMethodPtr> methods)
      : Node({}), methods(std::move(methods)) {}
  void accept(Visitor *visitor) override { visitor->visitMethodList(*this); }
  void acceptChildren(Visitor *visitor) override {
    std::sort(methods.begin(), methods.end(), SortUniquePtrPred());
    for (auto &e : methods) {
      visitor->visitRegularMethod(*e);
    }
  }

  void sortMethods() {
    std::stable_sort(methods.begin(), methods.end(), ast::SortUniquePtrPred());
  }
};

class MainMethodList : public Node {
public:
  std::vector<MainMethodPtr> mainMethods;

  MainMethodList(std::vector<MainMethodPtr> mainMethods)
      : Node({}), mainMethods(std::move(mainMethods)) {}
  void accept(Visitor *visitor) override {
    visitor->visitMainMethodList(*this);
  }
  void acceptChildren(Visitor *visitor) override {
    std::sort(mainMethods.begin(), mainMethods.end(), SortUniquePtrPred());
    for (auto &e : mainMethods) {
      visitor->visitMainMethod(*e);
    }
  }
};

class Class : public Node {
  std::string name;
  FieldList fields;
  MethodList methods;
  MainMethodList mainMethods;

public:
  Class(SourceLocation loc, std::string name, FieldList fields,
        MethodList methods, MainMethodList mainMethods)
      : Node(loc), name(std::move(name)), fields(std::move(fields)),
        methods(std::move(methods)), mainMethods(std::move(mainMethods)) {}

  void accept(Visitor *visitor) override { visitor->visitClass(*this); }

  void acceptChildren(Visitor *visitor) override {
    visitor->visitFieldList(fields);
    visitor->visitMethodList(methods);
    visitor->visitMainMethodList(mainMethods);
  }

  void sortMembers() {
    fields.sortFields();
    methods.sortMethods();
  }

  const FieldList *getFields() const { return &fields; }
  const MethodList *getMethods() const { return &methods; }
  const MainMethodList *getMainMethods() const { return &mainMethods; }

  const std::string &getName() const { return name; }

  bool operator<(const Class &o) const { return name < o.name; }
  bool operator==(const Class &o) const { return name == o.name; }
};

class Program : public Node {
  std::vector<ClassPtr> classes;

public:
  Program(SourceLocation loc, std::vector<ClassPtr> classes)
      : Node(loc), classes(std::move(classes)) {}

  void accept(Visitor *visitor) override { visitor->visitProgram(*this); }

  void acceptChildren(Visitor *visitor) override {
    std::sort(classes.begin(), classes.end(), SortUniquePtrPred());
    for (auto &cp : classes) {
      visitor->visitClass(*cp);
    }
  }

  std::vector<ClassPtr> &getClasses() { return classes; }
};
using ProgramPtr = std::unique_ptr<Program>;

class VariableDeclaration : public BlockStatement,
                            public SymbolTable::Definition {
  TypePtr type;
  SymbolTable::Symbol &symbol;
  // might be nullptr
  ExprPtr initializer;

public:
  VariableDeclaration(SourceLocation loc, TypePtr type,
                      SymbolTable::Symbol &sym, ExprPtr initializer)
      : BlockStatement(std::move(loc)), type(std::move(type)), symbol(sym),
        initializer(std::move(initializer)) {}

  void accept(Visitor *visitor) override {
    visitor->visitVariableDeclaration(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    type->accept(visitor);
    if (initializer != nullptr)
      initializer->accept(visitor);
  }

  SymbolTable::Symbol &getSymbol() const override { return symbol; }
  const std::string &getName() const { return symbol.name; }
  Type *getType() const override { return type.get(); }
  Expression *getInitializer() const { return initializer.get(); }
};

class PrimaryExpression : public Expression {
protected:
  PrimaryExpression(SourceLocation loc) : Expression(std::move(loc)) {}
};

class PrimaryRValueExpression : public PrimaryExpression {
protected:
  PrimaryRValueExpression(SourceLocation loc)
      : PrimaryExpression(std::move(loc)) {}
};

class NewArrayExpression : public PrimaryRValueExpression {
  ArrayTypePtr arrayType;
  ExprPtr size;

public:
  NewArrayExpression(SourceLocation loc, ArrayTypePtr arrayType, ExprPtr size)
      : PrimaryRValueExpression(std::move(loc)),
        arrayType(std::move(arrayType)), size(std::move(size)) {}

  void accept(Visitor *visitor) override {
    visitor->visitNewArrayExpression(*this);
  }
  void acceptChildren(Visitor *visitor) override {
    arrayType->accept(visitor);
    size->accept(visitor);
  }

  ArrayType *getArrayType() const { return arrayType.get(); }
  Expression *getSize() const { return size.get(); }
};

class NewObjectExpression : public PrimaryRValueExpression {
  std::string name;
  Class *classDef = nullptr;

public:
  NewObjectExpression(SourceLocation loc, std::string name)
      : PrimaryRValueExpression(std::move(loc)), name(std::move(name)) {}

  void accept(Visitor *visitor) override {
    visitor->visitNewObjectExpression(*this);
  }

  const std::string &getName() const { return name; }

  void setDef(Class *def) { classDef = def; }
  Class *getDef() const { return classDef; }
};

class LiteralExpression : public PrimaryRValueExpression {
public:
  LiteralExpression(SourceLocation loc)
      : PrimaryRValueExpression(std::move(loc)) {}
};

class IntLiteral : public LiteralExpression {
  int32_t value;

public:
  IntLiteral(SourceLocation loc, int32_t value)
      : LiteralExpression(std::move(loc)), value(value) {}

  void accept(Visitor *visitor) override { visitor->visitIntLiteral(*this); }

  int32_t getValue() const { return value; }
};

class BoolLiteral : public LiteralExpression {
  bool value;

public:
  BoolLiteral(SourceLocation loc, bool val)
      : LiteralExpression(std::move(loc)), value(val) {}

  void accept(Visitor *visitor) override { visitor->visitBoolLiteral(*this); }

  bool getValue() const { return value; }
};

class NullLiteral : public LiteralExpression {
public:
  NullLiteral(SourceLocation loc) : LiteralExpression(std::move(loc)) {}

  void accept(Visitor *visitor) override { visitor->visitNullLiteral(*this); }
};

class ThisLiteral : public LiteralExpression {
public:
  ThisLiteral(SourceLocation loc) : LiteralExpression(std::move(loc)) {}

  void accept(Visitor *visitor) override { visitor->visitThisLiteral(*this); }
};

class VarRef : public PrimaryExpression, public SymbolTable::Definition {
  SymbolTable::Symbol &symbol;
  SymbolTable::Definition *definition = nullptr;

public:
  VarRef(SourceLocation loc, SymbolTable::Symbol &sym)
      : PrimaryExpression(std::move(loc)), symbol(sym) {}

  void accept(Visitor *visitor) override { visitor->visitVarRef(*this); }

  SymbolTable::Symbol &getSymbol() const override { return symbol; }
  ast::Type *getType() const override { return nullptr; /* TODO */ }
  const std::string &getName() const { return symbol.name; }

  void setDef(SymbolTable::Definition *def) { definition = def; }
  SymbolTable::Definition *getDef() const { return definition; }
};

class MethodInvocation : public RValueExpression {
  ExprPtr left; // set to this if call from within function
  std::string name;
  // might be empty
  std::vector<ExprPtr> arguments;
  RegularMethod *methodDef;

public:
  MethodInvocation(SourceLocation loc, ExprPtr lhs, std::string methodName,
                   ExprList methodArgs)
      : RValueExpression(std::move(loc)), left(std::move(lhs)),
        name(std::move(methodName)), arguments(std::move(methodArgs)) {}

  void accept(Visitor *visitor) override {
    visitor->visitMethodInvocation(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    if (left != nullptr)
      left->accept(visitor);

    for (auto &arg : arguments) {
      arg->accept(visitor);
    }
  }

  std::vector<Expression *> getArguments() {
    std::vector<Expression *> result;
    for (std::vector<ExprPtr>::size_type i = 0; i < arguments.size(); i++) {
      result.push_back(arguments[i].get());
    }
    return result;
  }

  Expression *getLeft() const { return left.get(); }
  const std::string &getName() const { return name; }

  void setDef(RegularMethod *def) { methodDef = def; }
  RegularMethod *getDef() const { return methodDef; }
};

class FieldAccess : public Expression {
  // left.field_name
  ExprPtr left; // set to this if call from within function
  std::string name;
  Field *fieldDef;

public:
  FieldAccess(SourceLocation loc, ExprPtr lhs, std::string memberName)
      : Expression(std::move(loc)), left(std::move(lhs)),
        name(std::move(memberName)) {}

  void accept(Visitor *visitor) override { visitor->visitFieldAccess(*this); }
  void acceptChildren(Visitor *visitor) override {
    if (left != nullptr)
      left->accept(visitor);
  }

  Expression *getLeft() const { return left.get(); }
  const std::string &getName() const { return name; }

  void setDef(Field *def) { fieldDef = def; }
  Field *getDef() const { return fieldDef; }
};

class ArrayAccess : public Expression {
  ExprPtr array;
  ExprPtr index;

public:
  ArrayAccess(SourceLocation loc, ExprPtr lhs, ExprPtr index)
      : Expression(std::move(loc)), array(std::move(lhs)),
        index(std::move(index)) {}

  void accept(Visitor *visitor) override { visitor->visitArrayAccess(*this); }
  void acceptChildren(Visitor *visitor) override {
    array->accept(visitor);
    index->accept(visitor);
  }

  Expression *getArray() const { return array.get(); }
  Expression *getIndex() const { return index.get(); }
};

class BinaryExpression : public RValueExpression {

public:
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
  };

private:
  ExprPtr left;
  ExprPtr right;
  Op operation;

public:
  BinaryExpression(SourceLocation loc, ExprPtr lhs, ExprPtr rhs, Op op)
      : RValueExpression(std::move(loc)), left(std::move(lhs)),
        right(std::move(rhs)), operation(op) {}

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

  void accept(Visitor *visitor) override {
    visitor->visitBinaryExpression(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    left->accept(visitor);
    right->accept(visitor);
  }

  Expression *getLeft() const { return left.get(); }
  Expression *getRight() const { return right.get(); }
  Op getOperation() const { return operation; }
};

class UnaryExpression : public RValueExpression {
public:
  enum class Op { Not, Neg, None };

private:
  ExprPtr expression;
  Op operation;

public:
  UnaryExpression(SourceLocation loc, ExprPtr expression, Op operation)
      : RValueExpression(std::move(loc)), expression(std::move(expression)),
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
  void accept(Visitor *visitor) override {
    visitor->visitUnaryExpression(*this);
  }

  void acceptChildren(Visitor *visitor) override {
    expression->accept(visitor);
  }

  Expression *getExpression() const { return expression.get(); }
  Op getOperation() const { return operation; }
};

// ----

class DummyDefinition : public SymbolTable::Definition {
  static SymbolTable::Symbol dummySymbol;

public:
  virtual SymbolTable::Symbol &getSymbol() const { return dummySymbol; }
  virtual ast::Type *getType() const {
    assert(false);
    return nullptr;
  }
};

class DummySystemOut : public Field {
  static SymbolTable::Symbol dummySymbol;

public:
  DummySystemOut() : Field({}, nullptr, dummySymbol) {}
};

// ----

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
