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

public:
  FirmVisitor();
  virtual ~FirmVisitor() {};

  void visitProgram(ast::Program &program) override;
  void visitMainMethod(ast::MainMethod &method) override;
  void visitRegularMethod(ast::RegularMethod &method) override;
  void visitClass(ast::Class &klass) override;
};

#endif

