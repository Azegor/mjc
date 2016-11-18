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
  decl.acceptChildren(this);
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
  } else {
    // Look at local variables first
    if (auto decl = dynamic_cast<ast::VariableDeclaration*>(def)) {
      auto t = decl->getType();
      varRef.targetType.setFromAstType(t);
    } else if (auto decl = dynamic_cast<ast::Parameter*>(def)) {
      auto t = decl->getType();
      varRef.targetType.setFromAstType(t);
    } else if (auto decl = dynamic_cast<ast::Field*>(def)) {
      auto t = decl->getType();
      varRef.targetType.setFromAstType(t);
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

void SemanticVisitor::visitIntLiteral(ast::IntLiteral &lit) {
  lit.targetType.setInt();
}
void SemanticVisitor::visitBoolLiteral(ast::BoolLiteral &lit) {
  lit.targetType.setBool();

}
void SemanticVisitor::visitNullLiteral(ast::NullLiteral &lit) {
  lit.targetType.setNull();
}
void SemanticVisitor::visitThisLiteral(ast::ThisLiteral &lit) {
  assert (this->currentClass != nullptr);
  lit.targetType.setClass(this->currentClass->getName());
}

void SemanticVisitor::visitBinaryExpression(ast::BinaryExpression &expr) {
  expr.acceptChildren(this);

  switch(expr.getOperation()) {
    case ast::BinaryExpression::Op::Plus:
    case ast::BinaryExpression::Op::Minus:
    case ast::BinaryExpression::Op::Mul:
    case ast::BinaryExpression::Op::Div:
    case ast::BinaryExpression::Op::Mod:
    case ast::BinaryExpression::Op::Less:
    case ast::BinaryExpression::Op::LessEquals:
    case ast::BinaryExpression::Op::Greater:
    case ast::BinaryExpression::Op::GreaterEquals:
      if (!expr.getRight()->targetType.isInt() ||
          !expr.getLeft()->targetType.isInt()) {
        error(expr, "operands must both be int");
      }
      expr.targetType.setInt();
      break;

    case ast::BinaryExpression::Op::Or:
    case ast::BinaryExpression::Op::And:
      if (!expr.getRight()->targetType.isBool() ||
          !expr.getLeft()->targetType.isBool()) {
        error(expr, "operands must both be bool");
      }
      expr.targetType.setBool();
      break;

    case ast::BinaryExpression::Op::Assign:
      if (expr.getLeft()->targetType != expr.getRight()->targetType) {
        std::stringstream ss;
        ss << "Can't assign value of type " << expr.getRight()->targetType
           << " to variable of type " << expr.getLeft()->targetType;
        error(expr, ss.str());
      }
      expr.targetType = expr.getLeft()->targetType;
      break;

    default:
      assert(false);
  }
}
