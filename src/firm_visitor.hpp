#ifndef FIRM_VISITOR_H
#define FIRM_VISITOR_H

#include "ast.hpp"
#include "libfirm/firm.h"

struct FirmMethod {
  ir_type *type;
  size_t nParams;
  ir_node **params;
  FirmMethod(ir_type *type, size_t nParams, ir_node **params) :
    type(type), nParams(nParams), params(params) {}
};

class FirmVisitor : public ast::Visitor {
private:
  ir_type *intType;
  ir_type *boolType;
  ir_type *arrayType;

  // System.out.println special case
  ir_type *sysoutType;
  ir_entity *sysoutEntity;

  ir_type *currentClassType = nullptr;

  ast::Method *currentMethod = nullptr;

  std::unordered_map<ast::Class *, ir_type *> classTypes;
  std::unordered_map<ast::Method *, FirmMethod> methods;

  std::vector<ir_node*> nodeStack;

  ir_type *getIrType(ast::Type *type) {
    auto sType = type->getSemaType();
    switch (sType.kind) {
    case sem::TypeKind::Int:
      return this->intType;
    case sem::TypeKind::Bool:
      return this->boolType;
    case sem::TypeKind::Array:
      return this->arrayType;
    case sem::TypeKind::Class: {
      auto ct = dynamic_cast<ast::ClassType *>(type);
      return this->classTypes[ct->getDef()];
    }
    default:
      assert(false);
      return nullptr; // slience compiler warning
    }
  }

  void pushNode(ir_node* node) {
    nodeStack.push_back(node);
  }

  ir_node* popNode() {
    assert(nodeStack.size() > 0);
    ir_node *n = nodeStack[nodeStack.size() - 1];
    nodeStack.erase(nodeStack.end() - 1);

    return n;
  }

public:
  FirmVisitor();
  virtual ~FirmVisitor() {
    for (auto& e : classTypes) {
      free_type(e.second);
    }
    for (auto& e : methods) {
      delete[] e.second.params;
    }
    free_type(arrayType);
    free_type(boolType);
    free_type(intType);

    ir_finish();
  };

  void visitProgram(ast::Program &program) override;
  void visitMainMethod(ast::MainMethod &method) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
  void visitClass(ast::Class &klass) override;
  void visitReturnStatement(ast::ReturnStatement &stmt) override;
  void visitMethodInvocation(ast::MethodInvocation &invocation) override;
  void visitBoolLiteral(ast::BoolLiteral &lit) override;
  void visitIntLiteral(ast::IntLiteral &lit) override;
  void visitBinaryExpression(ast::BinaryExpression &expr) override;
  void visitVarRef(ast::VarRef &ref) override;
  void visitUnaryExpression(ast::UnaryExpression &expr) override;
};

#endif
