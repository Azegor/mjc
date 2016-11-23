#ifndef FIRM_VISITOR_H
#define FIRM_VISITOR_H

#include "ast.hpp"
#include "libfirm/firm.h"


class FirmVisitor : public ast::Visitor {
private:
  ir_type *intType;
  ir_type *boolType;

  ir_type *currentClassType = nullptr;

  std::unordered_map<ast::Class*, ir_type*> classTypes;

  ir_type *getIrType(ast::Type *type) {
    auto sType = type->getSemaType();

    if (sType.isInt())
      return this->intType;
    else if (sType.isBool())
      return this->boolType;
    else if (sType.isClass()) {
      auto ct = dynamic_cast<ast::ClassType*>(type);
      return this->classTypes[ct->getDef()];
    } else
      assert(false);
  }

public:
  FirmVisitor();
  virtual ~FirmVisitor() {};

  void visitProgram(ast::Program &program) override;
  void visitMainMethod(ast::MainMethod &method) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
  void visitClass(ast::Class &klass) override;
};

#endif

