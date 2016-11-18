#include "ast.hpp"

namespace sem {
void Type::setFromAstType(ast::Type *astType) {
  if (auto t = dynamic_cast<ast::PrimitiveType *>(astType)) {
    if (t->getType() == ast::PrimitiveType::TypeType::Boolean)
      setBool();
    else if (t->getType() == ast::PrimitiveType::TypeType::Int)
      setInt();
    else if (t->getType() == ast::PrimitiveType::TypeType::Void)
      setVoid();
    else
      assert(false);
  } else if (auto t = dynamic_cast<ast::ClassType *>(astType)) {
    setClass(t->getName());
  } else if (auto t = dynamic_cast<ast::ArrayType *>(astType)) {
    TypeKind innerKind = TypeKind::Unresolved;
    auto arrType = t->getElementType();
    std::string name;
    if (auto p = dynamic_cast<ast::ClassType *>(arrType)) {
      innerKind = TypeKind::Class;
      name = p->getName();
    } else if (auto p = dynamic_cast<ast::PrimitiveType *>(arrType)) {
      if (p->getType() == ast::PrimitiveType::TypeType::Boolean)
        innerKind = TypeKind::Bool;
      else if (p->getType() == ast::PrimitiveType::TypeType::Int)
        innerKind = TypeKind::Int;
      else
        assert(false);
    }
    setArray(innerKind, t->getDimension(), name);
  }
}

bool Type::conformsToAstType(ast::Type *astType) {
  if (auto t = dynamic_cast<ast::PrimitiveType *>(astType)) {
    switch (this->kind) {
    case TypeKind::Int:
      return t->getType() == ast::PrimitiveType::TypeType::Int;
    case TypeKind::Bool:
      return t->getType() == ast::PrimitiveType::TypeType::Boolean;
    case TypeKind::Void:
      return t->getType() == ast::PrimitiveType::TypeType::Void;
    default:
      return false;
    }
  } else if (auto t = dynamic_cast<ast::ClassType*>(astType)) {
    return (kind == TypeKind::Class &&
            name == t->getName()) ||
            kind == TypeKind::Null;
  } else if (auto t = dynamic_cast<ast::ArrayType*>(astType)) {
    (void) t;
    // TODO: Check element type + dimension (+ name?)
    return kind == TypeKind::Array;
  } else {
    std::cout << __FUNCTION__ << ": Unhandled astType" << std::endl;
  }

  return false;
}
}

static const char* typeKindToString(sem::TypeKind kind) {
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
    return o << typeKindToString(t.kind) << "(" <<  t.name << ")";
  case sem::TypeKind::Array:
    o << typeKindToString(t.innerKind);
    for (int i = 0; i < t.dimension; i ++)
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
