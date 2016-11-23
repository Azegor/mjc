#ifndef FIRM_VISITOR_H
#define FIRM_VISITOR_H

#include "ast.hpp"
#include "libfirm/firm.h"

class FirmVisitor : public ast::Visitor {
private:
  ir_type *intType;
  ir_type *boolType;

  ir_type *currentClassType = nullptr;

  std::unordered_map<ast::Class *, ir_type *> classTypes;

  ir_type *getIrType(ast::Type *type) {
    auto sType = type->getSemaType();
    switch (sType.kind) {
    case sem::TypeKind::Int:
      return this->intType;
    case sem::TypeKind::Bool:
      return this->boolType;
    case sem::TypeKind::Class: {
      auto ct = dynamic_cast<ast::ClassType *>(type);
      return this->classTypes[ct->getDef()];
    }
    default:
      assert(false);
	  return nullptr; // slience compiler warning
    }
  }

public:
  FirmVisitor();
  virtual ~FirmVisitor() {
    free_type(boolType);
    free_type(intType);
  };

  void visitProgram(ast::Program &program) override;
  void visitMainMethod(ast::MainMethod &method) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
  void visitClass(ast::Class &klass) override;
};

#endif
