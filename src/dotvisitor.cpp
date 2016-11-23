
#include "dotvisitor.hpp"
#include <iostream>
#include <sstream>

void DotVisitor::visitProgram(ast::Program &program) {
  s << "digraph G {\n";
  s << "rankdir=LR\n"; // TODO worth considering
  // first add all class node names:
  for (auto &klass : program.getClasses()) {
    nodeNames[klass.get()] = newNodeName();
    for (auto &f : klass->getFields()->fields) {
      nodeNames[f.get()] = newNodeName();
    }
    for (auto &m : klass->getMethods()->methods) {
      nodeNames[m.get()] = newNodeName();
    }
  }
  program.acceptChildren(this);
  s << "}" << '\n';
  assert(this->nodeStack.size() == 0);
}

void DotVisitor::visitClass(ast::Class &klass) {
  this->parentNode = newNodeName();
  auto nodeLabel = "Class " + klass.getName();
  auto nodeName = toplevelDecl(&klass, nodeLabel);
  pushNode(nodeName);
  klass.acceptChildren(this);
  popNode();
}

void DotVisitor::visitField(ast::Field &field) {
  auto nodeLabel = "Field(" + field.getName() + ")";
  auto nodeName = nodeNames[&field];
  nodeDeclForExistingName(nodeName, nodeLabel);

  pushNode(nodeName);
  edgeLabel("Type");
  field.getType()->accept(this);
  popNode();
}

void DotVisitor::visitRegularMethod(ast::RegularMethod &method) {
  auto nodeLabel = "Method(" + method.getName() + ")";
  auto nodeName = nodeNames[&method];
  nodeDeclForExistingName(nodeName, nodeLabel);

  pushNode(nodeName);
  {
    edgeLabel("Return Type");
    method.getReturnType()->accept(this);

    auto params = method.getParameters();
    int param_index = 0;
    for (auto &p : params) {
      edgeLabel("Param " + std::to_string(param_index));
      p->accept(this);
      param_index++;
    }

    auto bodyNode = nodeDecl("Body");
    pushNode(bodyNode);
    method.getBlock()->accept(this);
    popNode();
  }
  popNode();
}

void DotVisitor::visitMainMethod(ast::MainMethod &method) {
  auto nodeLabel = "MainMethod(" + method.getName() + ")";
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  method.acceptChildren(this);
  popNode();
}

void DotVisitor::visitBlock(ast::Block &block) {
  auto stmts = block.getStatements();
  block.acceptChildren(this);
}

void DotVisitor::visitVariableDeclaration(ast::VariableDeclaration &decl) {
  auto nodeLabel = "Variable Declaration(" + decl.getName() + ")";
  auto nodeName = nodeDecl(nodeLabel, &decl);

  pushNode(nodeName);
  {
    edgeLabel("Type");
    decl.getType()->accept(this);

    if (decl.getInitializer()) {
      edgeLabel("Initializer");
      decl.getInitializer()->accept(this);
    }
  }
  popNode();
}

void DotVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  auto nodeLabel = "Return";
  auto nodeName = nodeDecl(nodeLabel, nullptr, SHAPE_BOX);

  pushNode(nodeName);
  stmt.acceptChildren(this);
  popNode();
}

void DotVisitor::visitVarRef(ast::VarRef &ident) {
  auto nodeLabel = "VarRef " + ident.getName();
  auto nodeName = nodeDecl(nodeLabel);
  auto targetNode = nodeNames.find(dynamic_cast<ast::Node *>(ident.getDef()));
  if (targetNode != nodeNames.end()) {
    weakEdgeToNode(nodeName, targetNode->second);
  }
}

static std::string
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

void DotVisitor::visitBinaryExpression(ast::BinaryExpression &expr) {
  std::string nodeLabel =
      "BinaryExpression(" + binaryOperationToString(expr.getOperation()) + ")";
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  expr.acceptChildren(this);
  popNode();
}

void DotVisitor::visitIntLiteral(ast::IntLiteral &lit) {
  auto nodeLabel = "Int(" + std::to_string(lit.getValue()) + ")";
  auto nodeName = nodeDecl(nodeLabel);
}

void DotVisitor::visitBoolLiteral(ast::BoolLiteral &lit) {
  auto nodeLabel = "Bool(" + std::to_string(lit.getValue()) + ")";
  auto nodeName = nodeDecl(nodeLabel);
}

void DotVisitor::visitNullLiteral(ast::NullLiteral &lit) {
  (void)lit;
  auto nodeLabel = "null";
  auto nodeName = nodeDecl(nodeLabel);
}

void DotVisitor::visitThisLiteral(ast::ThisLiteral &lit) {
  (void)lit;
  auto nodeLabel = "this";
  auto nodeName = nodeDecl(nodeLabel);
}

void DotVisitor::visitMethodInvocation(ast::MethodInvocation &invocation) {
  auto nodeLabel = "MethodInvocation(" + invocation.getName() + ")";
  auto nodeName = nodeDecl(nodeLabel);
  auto targetNode = nodeNames.find(invocation.getDef());
  if (targetNode != nodeNames.end()) {
    weakEdgeToNode(nodeName, targetNode->second);
  } else {
    std::cerr << "Method Decl not found!" << std::endl;
  }

  pushNode(nodeName);
  invocation.acceptChildren(this);
  popNode();
}

void DotVisitor::visitFieldAccess(ast::FieldAccess &access) {
  auto nodeLabel = "FieldAccess(" + access.getName() + ")";
  auto nodeName = nodeDecl(nodeLabel);
  auto targetNode = nodeNames.find(access.getDef());
  if (targetNode != nodeNames.end()) {
    weakEdgeToNode(nodeName, targetNode->second);
  } else {
    std::cerr << "Field Decl not found!" << std::endl;
  }

  pushNode(nodeName);
  access.acceptChildren(this);
  popNode();
}

void DotVisitor::visitIfStatement(ast::IfStatement &stmt) {
  auto nodeLabel("If");
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  {
    edgeLabel("Condition");
    stmt.getCondition()->accept(this);
    auto thenStmt = stmt.getThenStatement();
    if (thenStmt != nullptr) {
      pushNode(nodeDecl("Then"));
      thenStmt->accept(this);
      popNode();
    }

    auto elseStmt = stmt.getElseStatement();
    if (elseStmt != nullptr) {
      pushNode(nodeDecl("Else"));
      elseStmt->accept(this);
      popNode();
    }
  }
  popNode();
}

void DotVisitor::visitUnaryExpression(ast::UnaryExpression &expr) {
  std::string nodeLabel = "UnaryExpression(";
  switch (expr.getOperation()) {
  case ast::UnaryExpression::Op::Not:
    nodeLabel += "Not";
    break;
  case ast::UnaryExpression::Op::Neg:
    nodeLabel += "Neg";
  default:;
  }
  nodeLabel += ")";
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  expr.acceptChildren(this);
  popNode();
}

void DotVisitor::visitArrayAccess(ast::ArrayAccess &access) {
  auto nodeName = nodeDecl("ArrayAccess");

  pushNode(nodeName);
  {
    access.getArray()->accept(this);
    edgeLabel("Index");
    access.getIndex()->accept(this);
  }
  popNode();
}

void DotVisitor::visitNewObjectExpression(ast::NewObjectExpression &expr) {
  auto nodeLabel = "New(" + expr.getName() + ")";
  auto nodeName = nodeDecl(nodeLabel);
  auto targetNode = nodeNames.find(dynamic_cast<ast::Node *>(expr.getDef()));
  if (targetNode != nodeNames.end()) {
    weakEdgeToNode(nodeName, targetNode->second);
  }
}

void DotVisitor::visitNewArrayExpression(ast::NewArrayExpression &expr) {
  auto nodeLabel = "New Array";
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  {
    edgeLabel("Type");
    expr.getArrayType()->accept(this);
    edgeLabel("Size");
    expr.getSize()->accept(this);
  }
  popNode();
}

void DotVisitor::visitWhileStatement(ast::WhileStatement &stmt) {
  auto nodeLabel = "While";
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  {
    edgeLabel("Condition");
    stmt.acceptChildren(this);
  }
  popNode();
}

void DotVisitor::visitArrayType(ast::ArrayType &type) {
  auto nodeLabel =
      "ArrayType\nDimension: " + std::to_string(type.getDimension());
  auto nodeName = nodeDecl(nodeLabel);

  pushNode(nodeName);
  {
    edgeLabel("Type");
    type.getElementType()->accept(this);
  }
  popNode();
}

void DotVisitor::visitPrimitiveType(ast::PrimitiveType &type) {
  std::string nodeLabel;
  switch (type.getPrimType()) {
  case ast::PrimitiveType::PrimType::Boolean:
    nodeLabel = "boolean";
    break;
  case ast::PrimitiveType::PrimType::Int:
    nodeLabel = "int";
    break;
  case ast::PrimitiveType::PrimType::Void:
    nodeLabel = "void";
    break;
  default:
    assert(false);
  }

  nodeDecl(nodeLabel);
}

void DotVisitor::visitClassType(ast::ClassType &type) {
  auto nodeLabel = "ClassType(" + type.getName() + ")";
  auto nodeName = nodeDecl(nodeLabel);
  auto targetNode = nodeNames.find(dynamic_cast<ast::Node *>(type.getDef()));
  if (targetNode != nodeNames.end()) {
    weakEdgeToNode(nodeName, targetNode->second);
  } else {
    std::cerr << "no def found" << std::endl;
  }
}

void DotVisitor::visitParameter(ast::Parameter &param) {
  auto nodeName = nodeDecl("Param(" + param.getName() + ")", &param);

  pushNode(nodeName);
  param.getType()->accept(this);
  popNode();
}
