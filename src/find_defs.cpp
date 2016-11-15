#include "find_defs.hpp"

void FindDefsVisitor::visitProgram(ast::Program &program) {
  currentProgram = &program;
  program.acceptChildren(this);
}

void FindDefsVisitor::visitClass(ast::Class &klass) {
  currentClass = &klass;
  klass.acceptChildren(this);
}
