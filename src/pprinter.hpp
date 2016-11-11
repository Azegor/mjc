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
  std::ostream& stream;
  std::string indentWith;
  int indentLevel;
  bool requireParenthesis = false;

public:
  PrettyPrinterVisitor(std::ostream &stream, std::string indentWith) : stream(stream), indentWith(std::move(indentWith)), indentLevel(0) {} 

  void visitProgram(ast::Program &program) override {
    newline();
  }

  void visitClass(ast::Class &klass) override {
    newline();
    stream << "class " << klass.getName() << " {";
    indentLevel++;
    newline();
    klass.accept(this);
    indentLevel--;
    newline();
    stream << "}";
  }

  void visitField(ast::Field &field) override {
    newline();
    stream << "public ";
    field.getType()->accept(this);
    stream << " " << field.getName() << ";";
  }

  void visitMethod(ast::Method &method) override {
    newline();
    stream << "public ";
    method.getReturnType()->accept(this);
    stream << " " << method.getName() << "(";
    ast::ParameterList params = method.getParameters();
    if (params.size() >= 1) {
      params[0]->accept(this);
    }
    for(ast::ParameterList::size_type i = 1; i < params.size(); i++) {
      stream << ", ";
      params[i]->accept(this);
    }
    stream << ") ";
    method.getBlock()->accept(this);
  }

  void visitMainMethod(ast::MainMethod &mainMethod) { 
    newline();
    stream << "public static void " << mainMethod.getName() << "(String[] " << mainMethod.getArgName() << ") ";
    mainMethod.getBlock()->accept(this);
  }

  void visitParameter(ast::Parameter &parameter) override {
    parameter.getType()->accept(this);
    stream << " " << parameter.getName();
  }

  void visitPrimitiveType(ast::PrimitiveType &primitiveType) override {
    switch (primitiveType.getType()) {
    case ast::PrimitiveType::TypeType::Boolean:
      stream << "boolean";
      break;
    case ast::PrimitiveType::TypeType::Int:
      stream << "int";
      break;
    case ast::PrimitiveType::TypeType::Void:
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
    for(int i=0; i<dimension; i++) {
      stream << "[]";
    }
  }

  void visitBlock(ast::Block &block) override {
    stream << "{";
    if(block.getContainsNothingExceptOneSingleLonelyEmptyExpression()) {
      stream << " ";
    } else {
      indentLevel++;
      ast::BlockStmtList statements = block.getStatements();

      for(ast::BlockStmtList::size_type i = 0; i < statements.size(); i++) {
        if (statements[i] != nullptr) {
          //statements[i] is no EmptyStatement
          newline();
          statements[i]->accept(this);
        }
      }
      indentLevel--;
      newline();
    }
    stream << "}";
  }

  void visitVariableDeclaration(ast::VariableDeclaration &variableDeclartion) { 
    variableDeclartion.getType()->accept(this);
    stream << " " << variableDeclartion.getName();
    ast::ExprPtr initializer = variableDeclartion.getInitializer();
    if(initializer != nullptr) {
      stream << " = ";
      initializer->accept(this);
    }
    stream << ";";  
  }

  void visitExpressionStatement(ast::ExpressionStatement &exprStmt) {
    exprStmt.getExpression()->accept(this);
    stream << ";";
  }
  void visitIfStatement(ast::IfStatement &ifStatement) { 
    stream << "if (";
    ifStatement.getCondition()->accept(this);
    stream << ") ";
    ast::StmtPtr thenStatement = ifStatement.getThenStatement();
    if (thenStatement == nullptr) {
      stream << ";";
    } else {
      indentLevel++;
      thenStatement->accept(this);
      indentLevel--;
    }
    ast::StmtPtr elseStatement = ifStatement.getElseStatement();
    if (elseStatement != nullptr) {
      newline();
      stream << "else ";
      indentLevel++;
      elseStatement->accept(this);
      indentLevel--;
    }
  }
  void visitWhileStatement(ast::WhileStatement &whileStatement) { 
    stream << "while (";
    whileStatement.getCondition()->accept(this);
    stream << ") ";
    ast::StmtPtr statement = whileStatement.getStatement();
    if(statement == nullptr) {
      stream << ";";
    } else {
      indentLevel++;
      statement->accept(this);
      indentLevel--;
    }
  }
  void visitReturnStatement(ast::ReturnStatement &returnStatement) { 
    ast::ExprPtr expression = returnStatement.getExpression();
    if(expression == nullptr) {
      stream << "return;";
    } else {
      stream << "return ";
      expression->accept(this);
      stream << ";";
    }
  }

  void visitNewArrayExpression(ast::NewArrayExpression &newArrayExpression) {
    requireParenthesis = false;
    stream << "new ";
    newArrayExpression.getArrayType()->accept(this);
    //TODO: include size in ArrayType
    //currently new A[<expr>][][] gets displayed as new A[][][]
  }

  void visitNewObjectExpression(ast::NewObjectExpression &newObjectExpression) {
    requireParenthesis = false;
    stream << "new " << newObjectExpression.getName() << "()";
  }

  void visitIntLiteral(ast::IntLiteral &intLiteral) {
    requireParenthesis = false;
    stream << std::to_string(intLiteral.getValue());
  }

  void visitBoolLiteral(ast::BoolLiteral &boolLiteral) {
    requireParenthesis = false;
    stream << ( boolLiteral.getValue() ? "true" : "false" );
  }

  void visitNullLiteral(ast::NullLiteral &nullLiteral) {
    requireParenthesis = false;
    stream << "null";
  }

  void visitThisLiteral(ast::ThisLiteral &thisLiteral) {
    requireParenthesis = false;
    stream << "this";
  }

  void visitIdent(ast::Ident &ident) {
    requireParenthesis = false;
    stream << ident.getName();
  }

  void visitMethodInvocation(ast::MethodInvocation &methodInvocation) {
    methodInvocation.getLeft()->accept(this);
    stream << "." << methodInvocation.getName() << "(";
    std::vector<ast::ExprPtr> arguments = methodInvocation.getArguments();
    if(arguments.size() >= 1) {
      arguments[0]->accept(this);
    }  
    for(std::vector<ast::ExprPtr>::size_type i=1; i<arguments.size(); i++) {
      stream << ", ";
      arguments[i]->accept(this);
    }
    stream << ")";
  }

  void visitFieldAccess(ast::FieldAccess &fieldAccess) {
    fieldAccess.getLeft()->accept(this);
    stream << "." << fieldAccess.getName();      
  }

  void visitArrayAccess(ast::ArrayAccess &arrayAccess) {
    arrayAccess.getArray()->accept(this);
    stream << "[";
    arrayAccess.getIndex()->accept(this);
    stream << "]";
  }

  void visitBinaryExpression(ast::BinaryExpression &binaryExpression) {
    maybePlaceParenthesis<ast::BinaryExpression>(binaryExpression, &PrettyPrinterVisitor::visitBinaryExpressionHelper);
  }

  void visitBinaryExpressionHelper(ast::BinaryExpression &binaryExpression) {
    if (binaryExpression.getOperation() != ast::BinaryExpression::Op::Assign) {
      requireParenthesis = true;
    }
    binaryExpression.getLeft()->accept(this);
    stream << " " << binaryOperationToString(binaryExpression.getOperation()) << " ";
    if (binaryExpression.getOperation() != ast::BinaryExpression::Op::Assign) {
      requireParenthesis = true;
    }
    binaryExpression.getRight()->accept(this);
  }

  void visitUnaryExpression(ast::UnaryExpression &unaryExpression) {
    maybePlaceParenthesis<ast::UnaryExpression>(unaryExpression, &PrettyPrinterVisitor::visitUnaryExpressionHelper);
  }

  void visitUnaryExpressionHelper(ast::UnaryExpression &unaryExpression) {
    stream << unaryOperationToString(unaryExpression.getOperation());
    requireParenthesis = true;
    unaryExpression.getExpression()->accept(this);
  }

  template <typename Et>
  void maybePlaceParenthesis(Et &expr, void (PrettyPrinterVisitor::*function)(Et &expression)) {
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

  static std::string binaryOperationToString(ast::BinaryExpression::Op operation) {
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
      return "%%";
    default:
      return "NONE";
    }
  }

  static std::string unaryOperationToString(ast::UnaryExpression::Op operation) {
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