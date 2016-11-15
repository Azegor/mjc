#ifndef FIND_DEFS_H
#define FIND_DEFS_H

#include "ast.hpp"

class FindDefsVisitor : public ast::Visitor {
  ast::Program* currentProgram = nullptr;
  ast::Class* currentClass = nullptr;
public:
  FindDefsVisitor() {}

  void visitProgram(ast::Program & program) override;
  void visitClass(ast::Class &klass) override;
//   void visitField(ast::Field &field) override;
//   void visitMethod(ast::Method &method) override;
//   void visitBlock(ast::Block &block) override;
//   void visitVariableDeclaration(ast::VariableDeclaration &decl) override;
//   void visitReturnStatement(ast::ReturnStatement &stmt) override;
//   void visitVarRef(ast::VarRef &ident) override;
//   void visitBinaryExpression(ast::BinaryExpression &expr) override;
//   void visitIntLiteral(ast::IntLiteral &lit) override;
//   void visitBoolLiteral(ast::BoolLiteral &lit) override;
//   void visitNullLiteral(ast::NullLiteral &lit) override;
//   void visitThisLiteral(ast::ThisLiteral &lit) override;
//   void visitMainMethod(ast::MainMethod &mainMethod) override;
//   void visitMethodInvocation(ast::MethodInvocation &invocation) override;
//   void visitFieldAccess(ast::FieldAccess &access) override;
//   void visitIfStatement(ast::IfStatement &ifStatement) override;
//   void visitUnaryExpression(ast::UnaryExpression &expr) override;
//   void visitArrayAccess(ast::ArrayAccess &access) override;
//   void visitNewObjectExpression(ast::NewObjectExpression &expr) override;
//   void visitNewArrayExpression(ast::NewArrayExpression &expr) override;
//   void visitWhileStatement(ast::WhileStatement &stmt) override;
//   void visitArrayType(ast::ArrayType &type) override;
//   void visitPrimitiveType(ast::PrimitiveType &type) override;
//   void visitClassType(ast::ClassType &type) override;
//   void visitParameter(ast::Parameter &param) override;

  virtual ~FindDefsVisitor() {}
};

#endif
