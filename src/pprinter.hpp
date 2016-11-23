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

#ifndef PPRINTER_H
#define PPRINTER_H

#include "ast.hpp"

class PrettyPrinterVisitor : public ast::Visitor {
  std::ostream &stream;
  std::string indentWith;
  int indentLevel;
  bool requireParenthesis = false;

public:
  PrettyPrinterVisitor(std::ostream &stream, std::string indentWith)
      : stream(stream), indentWith(std::move(indentWith)), indentLevel(0) {}

  void visitProgram(ast::Program &program) override {
    program.acceptChildren(this);
  }

  void visitClass(ast::Class &klass) override {
    stream << "class " << klass.getName() << " {";
    indentLevel++;
    klass.acceptChildren(this);
    indentLevel--;
    newline();
    stream << "}";
    newline();
  }

  void visitFieldList(ast::FieldList &fieldList) override {
    newline();
    stream << "/* fields: */";
    fieldList.acceptChildren(this);
  }

  void visitMethodList(ast::MethodList &methodList) override {
    stream << "\n"; // extra new line
    newline();
    stream << "/* methods: */";
    methodList.acceptChildren(this);
  }

  void visitMainMethodList(ast::MainMethodList &mainMethodList) override {
    stream << "\n"; // extra new line
    newline();
    stream << "/* main methods: */";
    mainMethodList.acceptChildren(this);
  }

  void visitField(ast::Field &field) override {
    newline();
    stream << "public ";
    field.getType()->accept(this);
    stream << " " << field.getName() << ";";
  }

  void visitRegularMethod(ast::RegularMethod &method) override {
    newline();
    stream << "public ";
    method.getReturnType()->accept(this);
    stream << " " << method.getName() << "(";
    std::vector<ast::Parameter *> params = method.getParameters();
    if (params.size() >= 1) {
      params[0]->accept(this);
    }
    for (std::vector<ast::Parameter *>::size_type i = 1; i < params.size();
         i++) {
      stream << ", ";
      params[i]->accept(this);
    }
    stream << ") ";
    method.getBlock()->accept(this);
  }

  void visitMainMethod(ast::MainMethod &mainMethod) override {
    newline();
    stream << "public static void " << mainMethod.getName() << "(String[] "
           << mainMethod.getArgName() << ") ";
    mainMethod.getBlock()->accept(this);
  }

  void visitParameter(ast::Parameter &parameter) override {
    parameter.getType()->accept(this);
    stream << " " << parameter.getName();
  }

  void visitPrimitiveType(ast::PrimitiveType &primitiveType) override {
    switch (primitiveType.getPrimType()) {
    case ast::PrimitiveType::PrimType::Boolean:
      stream << "boolean";
      break;
    case ast::PrimitiveType::PrimType::Int:
      stream << "int";
      break;
    case ast::PrimitiveType::PrimType::Void:
      stream << "void";
      break;
    default:
      stream << "NONE";
      break;
    }
  }

  void visitClassType(ast::ClassType &classType) override {
    stream << classType.getName();
  }

  void visitArrayType(ast::ArrayType &arrayType) override {
    arrayType.getElementType()->accept(this);
    int dimension = arrayType.getDimension();
    for (int i = 0; i < dimension; i++) {
      stream << "[]";
    }
  }

  void visitBlock(ast::Block &block) override {
    stream << "{";
    if (block.getContainsNothingExceptOneSingleLonelyEmptyExpression()) {
      stream << " ";
    } else {
      indentLevel++;
      std::vector<ast::BlockStatement *> statements = block.getStatements();

      for (std::vector<ast::BlockStatement *>::size_type i = 0;
           i < statements.size(); i++) {
        if (statements[i] != nullptr) {
          // statements[i] is no EmptyStatement
          newline();
          statements[i]->accept(this);
        }
      }
      indentLevel--;
      newline();
    }
    stream << "}";
  }

  void visitVariableDeclaration(
      ast::VariableDeclaration &variableDeclartion) override {
    variableDeclartion.getType()->accept(this);
    stream << " " << variableDeclartion.getName();
    ast::Expression *initializer = variableDeclartion.getInitializer();
    if (initializer != nullptr) {
      stream << " = ";
      initializer->accept(this);
    }
    stream << ";";
  }

  void visitExpressionStatement(ast::ExpressionStatement &exprStmt) override {
    exprStmt.getExpression()->accept(this);
    stream << ";";
  }
  void visitIfStatement(ast::IfStatement &ifStatement) override {
    stream << "if (";
    ifStatement.getCondition()->accept(this);
    stream << ") ";
    printStatementAsBlock(ifStatement.getThenStatement());
    printElseStatementAsBlock(ifStatement.getElseStatement());
  }

  void visitWhileStatement(ast::WhileStatement &whileStatement) override {
    stream << "while (";
    whileStatement.getCondition()->accept(this);
    stream << ") ";
    printStatementAsBlock(whileStatement.getStatement());
  }

  void printStatementAsBlock(ast::Statement *statement) {
    if (statement == nullptr) {
      stream << "{ }";
    } else if (dynamic_cast<ast::Block *>(statement)) {
      statement->accept(this);
    } else {
      stream << "{";
      indentLevel++;
      newline();
      statement->accept(this);
      indentLevel--;
      newline();
      stream << "}";
    }
  }

  void printElseStatementAsBlock(ast::Statement *statement) {
    if (statement == nullptr) {
      // do nothing
    } else if (ast::Block *b = dynamic_cast<ast::Block *>(statement)) {
      if (!b->getContainsNothingExceptOneSingleLonelyEmptyExpression()) {
        newline();
        stream << "else ";
        b->accept(this);
      }
    } else {
      newline();
      stream << "else {";
      indentLevel++;
      newline();
      statement->accept(this);
      indentLevel--;
      newline();
      stream << "}";
    }
  }

  void visitReturnStatement(ast::ReturnStatement &returnStatement) override {
    ast::Expression *expression = returnStatement.getExpression();
    if (expression == nullptr) {
      stream << "return;";
    } else {
      stream << "return ";
      expression->accept(this);
      stream << ";";
    }
  }

  void visitNewArrayExpression(
      ast::NewArrayExpression &newArrayExpression) override {
    requireParenthesis = false;
    stream << "new ";
    auto arrayType = newArrayExpression.getArrayType();
    arrayType->getElementType()->accept(this);
    int dimension = arrayType->getDimension();
    stream << "[";
    newArrayExpression.getSize()->accept(this);
    stream << "]";
    // start from dimension 1:
    for (int i = 1; i < dimension; i++) {
      stream << "[]";
    }
  }

  void visitNewObjectExpression(
      ast::NewObjectExpression &newObjectExpression) override {
    requireParenthesis = false;
    stream << "new " << newObjectExpression.getName() << "()";
  }

  void visitIntLiteral(ast::IntLiteral &intLiteral) override {
    maybePlaceParenthesis<ast::IntLiteral>(
        intLiteral, &PrettyPrinterVisitor::visitIntLiteralHelper);
  }

  void visitIntLiteralHelper(ast::IntLiteral &intLiteral) {
    requireParenthesis = intLiteral.getValue() < 0 ? true : false;
    stream << std::to_string(intLiteral.getValue());
  }

  void visitBoolLiteral(ast::BoolLiteral &boolLiteral) override {
    requireParenthesis = false;
    stream << (boolLiteral.getValue() ? "true" : "false");
  }

  void visitNullLiteral(ast::NullLiteral &) override {
    requireParenthesis = false;
    stream << "null";
  }

  void visitThisLiteral(ast::ThisLiteral &) override {
    requireParenthesis = false;
    stream << "this";
  }

  void visitVarRef(ast::VarRef &varRef) override {
    requireParenthesis = false;
    stream << varRef.getName();
  }

  void visitMethodInvocation(ast::MethodInvocation &methodInvocation) override {
    methodInvocation.getLeft()->accept(this);
    stream << "." << methodInvocation.getName() << "(";
    std::vector<ast::Expression *> arguments = methodInvocation.getArguments();
    if (arguments.size() >= 1) {
      arguments[0]->accept(this);
    }
    for (std::vector<ast::Expression *>::size_type i = 1; i < arguments.size();
         i++) {
      stream << ", ";
      arguments[i]->accept(this);
    }
    stream << ")";
  }

  void visitFieldAccess(ast::FieldAccess &fieldAccess) override {
    fieldAccess.getLeft()->accept(this);
    stream << "." << fieldAccess.getName();
  }

  void visitArrayAccess(ast::ArrayAccess &arrayAccess) override {
    arrayAccess.getArray()->accept(this);
    stream << "[";
    arrayAccess.getIndex()->accept(this);
    stream << "]";
  }

  void visitBinaryExpression(ast::BinaryExpression &binaryExpression) override {
    maybePlaceParenthesis<ast::BinaryExpression>(
        binaryExpression, &PrettyPrinterVisitor::visitBinaryExpressionHelper);
  }

  void visitBinaryExpressionHelper(ast::BinaryExpression &binaryExpression) {
    if (binaryExpression.getOperation() != ast::BinaryExpression::Op::Assign) {
      requireParenthesis = true;
    }
    binaryExpression.getLeft()->accept(this);
    stream << " " << binaryOperationToString(binaryExpression.getOperation())
           << " ";
    if (binaryExpression.getOperation() != ast::BinaryExpression::Op::Assign) {
      requireParenthesis = true;
    }
    binaryExpression.getRight()->accept(this);
  }

  void visitUnaryExpression(ast::UnaryExpression &unaryExpression) override {
    maybePlaceParenthesis<ast::UnaryExpression>(
        unaryExpression, &PrettyPrinterVisitor::visitUnaryExpressionHelper);
  }

  void visitUnaryExpressionHelper(ast::UnaryExpression &unaryExpression) {
    stream << unaryOperationToString(unaryExpression.getOperation());
    requireParenthesis = true;
    unaryExpression.getExpression()->accept(this);
  }

  template <typename Et>
  void maybePlaceParenthesis(
      Et &expr, void (PrettyPrinterVisitor::*function)(Et &expression)) {
    using namespace std::placeholders;
    auto fp = std::bind(function, this, _1);
    if (requireParenthesis) {
      requireParenthesis = false;
      stream << "(";
      fp(expr);
      stream << ")";
    } else {
      fp(expr);
    }
  }

  static const char *
  binaryOperationToString(ast::BinaryExpression::Op operation) {
    switch (operation) {
    case ast::BinaryExpression::Op::Assign:
      return "=";
    case ast::BinaryExpression::Op::Or:
      return "||";
    case ast::BinaryExpression::Op::And:
      return "&&";
    case ast::BinaryExpression::Op::Equals:
      return "==";
    case ast::BinaryExpression::Op::NotEquals:
      return "!=";
    case ast::BinaryExpression::Op::Less:
      return "<";
    case ast::BinaryExpression::Op::LessEquals:
      return "<=";
    case ast::BinaryExpression::Op::Greater:
      return ">";
    case ast::BinaryExpression::Op::GreaterEquals:
      return ">=";
    case ast::BinaryExpression::Op::Plus:
      return "+";
    case ast::BinaryExpression::Op::Minus:
      return "-";
    case ast::BinaryExpression::Op::Mul:
      return "*";
    case ast::BinaryExpression::Op::Div:
      return "/";
    case ast::BinaryExpression::Op::Mod:
      return "%";
    default:
      return "NONE";
    }
  }

  static std::string
  unaryOperationToString(ast::UnaryExpression::Op operation) {
    switch (operation) {
    case ast::UnaryExpression::Op::Neg:
      return "-";
    case ast::UnaryExpression::Op::Not:
      return "!";
    default:
      return "NONE";
    }
  }

  void newline() {
    stream << std::endl;
    for (int i = 0; i < indentLevel; i++) {
      stream << indentWith;
    }
  }
};

#endif // PPRINTER_H
