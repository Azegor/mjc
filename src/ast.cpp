#include "ast.hpp"

namespace ast {
void Visitor::visitProgram(Program &program) { program.acceptChildren(this); }
void Visitor::visitClass(Class &klass) { klass.acceptChildren(this); }
void Visitor::visitFieldList(FieldList &fieldList) {
  fieldList.acceptChildren(this);
}
void Visitor::visitMethodList(MethodList &methodList) {
  methodList.acceptChildren(this);
}
void Visitor::visitMainMethodList(MainMethodList &mainMethodList) {
  mainMethodList.acceptChildren(this);
}
void Visitor::visitField(Field &field) { field.acceptChildren(this); }
void Visitor::visitMethod(Method &method) { method.acceptChildren(this); }
void Visitor::visitMainMethod(MainMethod &mainMethod) {
  mainMethod.acceptChildren(this);
}
void Visitor::visitParameter(Parameter &parameter) {
  parameter.acceptChildren(this);
}
void Visitor::visitPrimitiveType(PrimitiveType &primitiveType) {
  primitiveType.acceptChildren(this);
}
void Visitor::visitClassType(ClassType &classType) {
  classType.acceptChildren(this);
}
void Visitor::visitArrayType(ArrayType &arrayType) {
  arrayType.acceptChildren(this);
}
void Visitor::visitBlock(Block &block) { block.acceptChildren(this); }
void Visitor::visitVariableDeclaration(
    VariableDeclaration &variableDeclaration) {
  variableDeclaration.acceptChildren(this);
}
void Visitor::visitExpressionStatement(ExpressionStatement &exprStmt) {
  exprStmt.acceptChildren(this);
}
void Visitor::visitIfStatement(IfStatement &ifStatement) {
  ifStatement.acceptChildren(this);
}
void Visitor::visitWhileStatement(WhileStatement &whileStatement) {
  whileStatement.acceptChildren(this);
}
void Visitor::visitReturnStatement(ReturnStatement &returnStatement) {
  returnStatement.acceptChildren(this);
}
void Visitor::visitNewArrayExpression(NewArrayExpression &newArrayExpression) {
  newArrayExpression.acceptChildren(this);
}
void Visitor::visitNewObjectExpression(
    NewObjectExpression &newObjectExpression) {
  newObjectExpression.acceptChildren(this);
}
void Visitor::visitIntLiteral(IntLiteral &intLiteral) {
  intLiteral.acceptChildren(this);
}
void Visitor::visitBoolLiteral(BoolLiteral &boolLiteral) {
  boolLiteral.acceptChildren(this);
}
void Visitor::visitNullLiteral(NullLiteral &nullLiteral) {
  nullLiteral.acceptChildren(this);
}
void Visitor::visitThisLiteral(ThisLiteral &thisLiteral) {
  thisLiteral.acceptChildren(this);
}
void Visitor::visitVarRef(VarRef &varRef) { varRef.acceptChildren(this); }
void Visitor::visitMethodInvocation(MethodInvocation &methodInvocation) {
  methodInvocation.acceptChildren(this);
}
void Visitor::visitFieldAccess(FieldAccess &fieldAccess) {
  fieldAccess.acceptChildren(this);
}
void Visitor::visitArrayAccess(ArrayAccess &arrayAccess) {
  arrayAccess.acceptChildren(this);
}
void Visitor::visitBinaryExpression(BinaryExpression &binaryExpression) {
  binaryExpression.acceptChildren(this);
}
void Visitor::visitUnaryExpression(UnaryExpression &unaryExpression) {
  unaryExpression.acceptChildren(this);
}

} /* namespace ast */
