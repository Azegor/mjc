#include "find_defs.hpp"

void FindDefsVisitor::visitProgram(ast::Program &program) {
  currentProgram = &program;
  checkForDuplicates(program.getClasses());
  program.acceptChildren(this);
}

void FindDefsVisitor::visitClass(ast::Class &klass) {
  currentClass = &klass;
  klass.acceptChildren(this);
}

void FindDefsVisitor::visitFieldList(ast::FieldList &fieldList) {
  checkForDuplicates(fieldList.fields);
  fieldList.acceptChildren(this);
}

void FindDefsVisitor::visitMethodList(ast::MethodList &methodList) {
  checkForDuplicates(methodList.methods);
  methodList.acceptChildren(this);
}

void FindDefsVisitor::visitMainMethodList(ast::MainMethodList &mainMethodList) {
  checkForDuplicates(mainMethodList.mainMethods);
  mainMethodList.acceptChildren(this);
}

void FindDefsVisitor::visitBlock(ast::Block &block) {
  symTbl.enterScope();
  block.acceptChildren(this);
  symTbl.leaveScope();
}

void FindDefsVisitor::visitVariableDeclaration(ast::VariableDeclaration &decl) {
  auto &sym = decl.getSymbol();
  if (symTbl.isDefinedInCurrentScope(sym)) {
    error(decl, "Variable '" + decl.getSymbol().name + "' already defined");
  }
  symTbl.insert(sym, &decl);
}

void FindDefsVisitor::visitVarRef(ast::VarRef &varRef) {
  auto *def = symTbl.lookup(varRef.getSymbol());
  if (!def) {
    error(varRef, "Unknown variable '" + varRef.getSymbol().name + "'");
  }
  varRef.setDef(def);
}
