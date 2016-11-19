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

#ifndef RETURN_VISITOR_H
#define RETURN_VISITOR_H

#include <string>

#include "ast.hpp"

class ReturnAnalysisVisitor : public ast::Visitor {
  std::string fileName;

  sem::Type currentReturnType;

public:
  ReturnAnalysisVisitor(std::string fileName) : Visitor(std::move(fileName)) {}

  void visitMethod(ast::Method &method) override;
  void visitMainMethod(ast::MainMethod &mainMethod) override;
  void visitBlock(ast::Block &block) override;
  void visitReturnStatement(ast::ReturnStatement &stmt) override;
  //   void visitProgram(ast::Program &program) override;
  //   void visitClass(ast::Class &klass) override;
  //   void visitFieldList(ast::FieldList &fieldList) override;
  //   void visitMethodList(ast::MethodList &methodList) override;
  //   void visitMainMethodList(ast::MainMethodList &mainMethodList) override;
  //   void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
  //   void visitVarRef(ast::VarRef &varRef) override;
  //   void visitParameter(ast::Parameter &param) override;
  //   void visitNewObjectExpression(ast::NewObjectExpression &expr) override;
  //   void visitClassType(ast::ClassType &type) override;
  //   void visitNewArrayExpression(ast::NewArrayExpression &expr) override;
  //   void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  //   void visitIfStatement(ast::IfStatement &ifStatement) override;
  //   void visitWhileStatement(ast::WhileStatement &stmt) override;
  //   void visitField(ast::Field &field) override;
  //   void visitBinaryExpression(ast::BinaryExpression &expr) override;
  //   void visitIntLiteral(ast::IntLiteral &lit) override;
  //   void visitBoolLiteral(ast::BoolLiteral &lit) override;
  //   void visitNullLiteral(ast::NullLiteral &lit) override;
  //   void visitThisLiteral(ast::ThisLiteral &lit) override;
  //   void visitFieldAccess(ast::FieldAccess &access) override;
  //   void visitUnaryExpression(ast::UnaryExpression &expr) override;
  //   void visitArrayAccess(ast::ArrayAccess &access) override;
  //   void visitArrayType(ast::ArrayType &type) override;
  //   void visitPrimitiveType(ast::PrimitiveType &type) override;
};

#endif // RETURN_VISITOR_H
