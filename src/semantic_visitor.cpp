#include "semantic_visitor.hpp"

void SemanticVisitor::visitProgram(ast::Program &program) {
  currentProgram = &program;
  checkForDuplicates(program.getClasses());
  program.acceptChildren(this);
}

void SemanticVisitor::visitClass(ast::Class &klass) {
  currentClass = &klass;
  klass.acceptChildren(this);
}

void SemanticVisitor::visitFieldList(ast::FieldList &fieldList) {
  checkForDuplicates(fieldList.fields);
  for (auto &f : fieldList.fields) {
    symTbl.insert(f->getSymbol(), f.get());
  }
  fieldList.acceptChildren(this);
}

void SemanticVisitor::visitMethodList(ast::MethodList &methodList) {
  checkForDuplicates(methodList.methods);
  methodList.acceptChildren(this);
}

void SemanticVisitor::visitMainMethodList(ast::MainMethodList &mainMethodList) {
  checkForDuplicates(mainMethodList.mainMethods);
  mainMethodList.acceptChildren(this);
}

void SemanticVisitor::visitMethod(ast::Method &method) {
  symTbl.enterScope(); // for parameters
  method.acceptChildren(this);
  symTbl.leaveScope();
}

void SemanticVisitor::visitBlock(ast::Block &block) {
  // TODO: this currently adds an extra block for the outmost block,
  // the parameters therefore being alone in the outpust block.
  // since shadowing is forbidden this should be no problem
  symTbl.enterScope();
  block.acceptChildren(this);
  symTbl.leaveScope();
}

void SemanticVisitor::visitVariableDeclaration(ast::VariableDeclaration &decl) {
  auto &sym = decl.getSymbol();
  if (symTbl.isDefinedInCurrentScope(sym)) {
    error(decl, "Variable '" + decl.getSymbol().name + "' already defined");
  }
  auto prev = symTbl.lookup(sym);
  if (prev && dynamic_cast<ast::Field *>(prev) == nullptr) {
    error(decl, "Variable '" + decl.getSymbol().name + "' already defined");
  }
  symTbl.insert(sym, &decl);
  decl.acceptChildren(this);
}

void SemanticVisitor::visitVarRef(ast::VarRef &varRef) {
  auto *def = symTbl.lookup(varRef.getSymbol());
  if (!def) {
    if (varRef.getName() == "System") {
      // TODO
    } else {
      error(varRef, "Unknown variable '" + varRef.getSymbol().name + "'");
    }
  }
  varRef.setDef(def);
  varRef.acceptChildren(this);
}

void SemanticVisitor::visitParameter(ast::Parameter &param) {
  auto &sym = param.getSymbol();
  if (symTbl.isDefinedInCurrentScope(sym)) {
    error(param, "Parameter '" + param.getSymbol().name + "' already defined");
  }
  symTbl.insert(sym, &param);
  param.acceptChildren(this);
}

void SemanticVisitor::visitNewObjectExpression(ast::NewObjectExpression &expr) {
  auto *def = findClassByName(expr.getName());
  if (!def) {
    error(expr, "Undefined class '" + expr.getName() + "'");
  }
  expr.setDef(def);
  expr.acceptChildren(this);
}

void SemanticVisitor::visitClassType(ast::ClassType &type) {
  auto *def = findClassByName(type.getName());
  if (!def) {
    error(type, "Undefined class '" + type.getName() + "'");
  }
  type.setDef(def);
  type.acceptChildren(this);
}
