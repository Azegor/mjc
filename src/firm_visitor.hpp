#ifndef FIRM_VISITOR_H
#define FIRM_VISITOR_H

#include "ast.hpp"
#include "libfirm/firm.h"


class FirmVisitor : public ast::Visitor {
private:
  ir_type *intType;
  ir_type *boolType;
  ir_type *voidType;

  ir_type *currentClassType = nullptr;

  std::unordered_map<ast::Class*, ir_type*> classTypes;

  ir_type *getIrType(ast::Type *type) {
    if (auto t = dynamic_cast<ast::ClassType*>(type)) {
      return this->classTypes[t->getDef()];
    } else if (auto t = dynamic_cast<ast::PrimitiveType*>(type)) {
      if (t->getPrimType() == ast::PrimitiveType::PrimType::Int)
        return this->intType;
      else if (t->getPrimType() == ast::PrimitiveType::PrimType::Boolean)
        return this->boolType;
      else
        assert(false);
    }
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

