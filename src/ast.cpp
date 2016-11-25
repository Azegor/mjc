#include "ast.hpp"

SymbolTable::Symbol ast::DummyDefinition::dummySymbol("<Dummy>");
SymbolTable::Symbol ast::DummySystemOut::dummySymbol("System.out");

static const char *typeKindToString(sem::TypeKind kind) {
  switch (kind) {
  case sem::TypeKind::Bool:
    return "bool";
  case sem::TypeKind::Int:
    return "int";
  case sem::TypeKind::Class:
    return "Class";
  case sem::TypeKind::Array:
    assert(false);
    return "";
  case sem::TypeKind::Void:
    return "void";
  case sem::TypeKind::Null:
    return "null";
  case sem::TypeKind::Unresolved:
    // Print these anyway for easier debugging
    return "Unresolved";
  }

  return "";
}

std::ostream &operator<<(std::ostream &o, const sem::Type &t) {
  switch (t.kind) {
  case sem::TypeKind::Bool:
  case sem::TypeKind::Int:
  case sem::TypeKind::Void:
  case sem::TypeKind::Null:
  case sem::TypeKind::Unresolved:
    return o << typeKindToString(t.kind);

  case sem::TypeKind::Class:
    return o << typeKindToString(t.kind) << "(" << t.name << ")";
  case sem::TypeKind::Array:
    o << typeKindToString(t.innerKind);
    if (t.innerKind == sem::TypeKind::Class) {
      o << "(" << t.name << ")";
    }
    for (int i = 0; i < t.dimension; i++)
      o << "[]";
    return o;
  }
  return o;
}

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
void Visitor::visitRegularMethod(RegularMethod &method) {
  method.acceptChildren(this);
}
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


std::vector<VariableDeclaration*> Block::countVariableDeclarations() {
  std::vector<VariableDeclaration*> decls;
  for (auto &stmt : statements) {
    if (auto d = dynamic_cast<VariableDeclaration*>(stmt.get())) {
      decls.push_back(d);
    }
  }

  return decls;
}

} /* namespace ast */
