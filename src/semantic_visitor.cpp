#include "semantic_visitor.hpp"

void SemanticVisitor::visitProgram(ast::Program &program) {
  currentProgram = &program;
  checkForDuplicates(program.getClasses());
  program.acceptChildren(this);

  if (!this->mainMethodFound) {
    error(program, "Program does not contain a valid main method");
  }
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

void SemanticVisitor::visitMainMethod(ast::MainMethod &mm) {
  mm.acceptChildren(this);

  if (mm.getName() == "main" &&
      mm.getArgName() == "args") {
    this->mainMethodFound = true;
  }
}

void SemanticVisitor::visitMethod(ast::Method &method) {
  symTbl.enterScope(); // for parameters
  currentMethod = &method;
  method.acceptChildren(this);
  symTbl.leaveScope();
  currentMethod = nullptr;
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

  if (decl.getInitializer() != nullptr &&
      !decl.getInitializer()->targetType.conformsToAstType(decl.getType())) {
    std::stringstream ss;
    ss << "Can't assign from " << decl.getInitializer()->targetType << " to "
       << "umm"; // TODO: Write toStr() or stream op for ast::Type variants
    error(decl, ss.str());
  }

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
    if (auto decl = dynamic_cast<ast::VariableDeclaration *>(def)) {
      auto t = decl->getType();
      varRef.targetType.setFromAstType(t);
    } else if (auto decl = dynamic_cast<ast::Parameter *>(def)) {
      auto t = decl->getType();
      varRef.targetType.setFromAstType(t);
    } else if (auto decl = dynamic_cast<ast::Field *>(def)) {
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

  expr.targetType.setClass(expr.getName());
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
  assert(this->currentClass != nullptr);
  lit.targetType.setClass(this->currentClass->getName());
}

void SemanticVisitor::visitBinaryExpression(ast::BinaryExpression &expr) {
  expr.acceptChildren(this);

  auto left  = expr.getLeft();
  auto right = expr.getRight();

  switch (expr.getOperation()) {
  case ast::BinaryExpression::Op::Plus:
  case ast::BinaryExpression::Op::Minus:
  case ast::BinaryExpression::Op::Mul:
  case ast::BinaryExpression::Op::Div:
  case ast::BinaryExpression::Op::Mod:
    if (!right->targetType.isInt() ||
        !left->targetType.isInt()) {
      error(expr, "operands must both be int");
    }
    expr.targetType.setInt();
    break;

  case ast::BinaryExpression::Op::Less:
  case ast::BinaryExpression::Op::LessEquals:
  case ast::BinaryExpression::Op::Greater:
  case ast::BinaryExpression::Op::GreaterEquals:
    if (!right->targetType.isInt() ||
        !left->targetType.isInt()) {
      error(expr, "operands must both be int");
    }
    expr.targetType.setBool();
    break;

  case ast::BinaryExpression::Op::Or:
  case ast::BinaryExpression::Op::And:
    if (!right->targetType.isBool() ||
        !left->targetType.isBool()) {
      error(expr, "operands must both be bool");
    }
    expr.targetType.setBool();
    break;

  case ast::BinaryExpression::Op::Assign:
    if (left->targetType != right->targetType) {
      std::stringstream ss;
      ss << "Can't assign value of type " << right->targetType
         << " to variable of type " << left->targetType;
      error(expr, ss.str());
    }
    expr.targetType = left->targetType;
    break;

  case ast::BinaryExpression::Op::Equals:
  case ast::BinaryExpression::Op::NotEquals:
    if (expr.getLeft()->targetType != expr.getRight()->targetType) {
      // class types of different classes can still be compared
      if (left->targetType.isClass() && right->targetType.isClass()) {
        // Fine.
        expr.targetType.setBool();
      } else {
        std::stringstream ss;
        ss << "Can't compare expression of type '" << left->targetType
           << "' to expression of type '" << right->targetType << "'";
        error(expr, ss.str());
      }
    } else {
      expr.targetType.setBool();
    }
    break;

  default:
    assert(false);
  }
}

void SemanticVisitor::visitNewArrayExpression(ast::NewArrayExpression &expr) {
  expr.acceptChildren(this);

  if (!expr.getSize()->targetType.isInt()) {
    error(*expr.getSize(), "Array indices must be ints");
  }

  expr.targetType.setArray(); // TODO: Proagate name (and dimension?)
}

void SemanticVisitor::visitFieldAccess(ast::FieldAccess &access) {
  access.acceptChildren(this);

  // Handle System.out.println separately here...
  if (auto ref = dynamic_cast<ast::VarRef*>(access.getLeft())) {
    if (ref->getName() == "System" && access.getName() == "out") {
      // System.out, .println might follow but don't further check this
      return;
    }
  }

  auto &lhsType = access.getLeft()->targetType;
  if (!lhsType.isClass()) {
    error(access, "Left hand side of field access must be class type object");
  }
  auto cls = findClassByName(lhsType.name);
  if (!cls) {
    error(*access.getLeft(), "Unknown class type'" + lhsType.name + '\'');
  }
}

void SemanticVisitor::visitMethodInvocation(ast::MethodInvocation &invocation) {
  invocation.acceptChildren(this);

  if (auto left = dynamic_cast<ast::VarRef *>(invocation.getLeft())) {
    auto *def = symTbl.lookup(left->getSymbol());
    // left has already been visited so we should never arrive here with null
    // definition
    assert(def != nullptr);
    auto defType = def->getType();
    if (auto cl = dynamic_cast<ast::ClassType *>(defType)) {
      auto *classDef = findClassByName(cl->getName());
      assert(classDef != nullptr);

      // Find method in class
      ast::Method *method = nullptr;
      for (auto &m : classDef->getMethods()->methods) {
        if (m->getName() == invocation.getName()) {
          method = m.get();
          break;
        }
      }

      if (method == nullptr) {
        error(invocation, "Class " + cl->getName() +
                              " does not contain a method " +
                              invocation.getName());
      }
      // TODO: Can we even be sure that the return type has been visited at this
      // point?
      // Valid method, valid class, propagate type!
      invocation.targetType.setFromAstType(method->getReturnType());
    } else {
      error(invocation, "Methods can only be invoked on class types");
    }
  } else if (auto left = dynamic_cast<ast::FieldAccess*>(invocation.getLeft())) {
    // System.out.println special case: VarRef(System) - FieldAccess(out) - MethodInvocation(println)
    if (invocation.getName() == "println") {
      auto system = dynamic_cast<ast::VarRef*>(left->getLeft());
      if (system != nullptr) {
        auto args = invocation.getArguments();
        if (args.size() != 1) {
          error(invocation, "System.out.println takes exactly one int argument, " +
                            std::to_string(args.size()) + " given");
        } else if (!args.at(0)->targetType.isInt()) {
          std::stringstream ss;
          ss << "System.out.println takes one int argument, but argument is of type " << args.at(0)->targetType;
          error(invocation, ss.str());
        }

        // Yep, System.out.println call
        return;
      }
    }
  } else {
    error(invocation, "Can't access method " + invocation.getName());
  }
}

void SemanticVisitor::visitIfStatement(ast::IfStatement &ifStatement) {
  ifStatement.acceptChildren(this);

  if (!ifStatement.getCondition()->targetType.isBool()) {
    std::stringstream ss;
    ss << "if condition must be boolean, not " << ifStatement.getCondition()->targetType;
    error(*ifStatement.getCondition(), ss.str());
  }
}

void SemanticVisitor::visitWhileStatement(ast::WhileStatement &stmt) {
  stmt.acceptChildren(this);

  if (!stmt.getCondition()->targetType.isBool()) {
    std::stringstream ss;
    ss << "while condition must be boolean, not " << stmt.getCondition()->targetType;
    error(*stmt.getCondition(), ss.str());
  }
}

void SemanticVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  stmt.acceptChildren(this);

  if (currentMethod == nullptr) {
    error(stmt, "Return statement outside of Method");
  }

  auto expr = stmt.getExpression();
  if (expr == nullptr)
    return;

  auto methodReturnType = this->currentMethod->getReturnType();

  if (!expr->targetType.conformsToAstType(methodReturnType)) {
    std::stringstream ss;
    ss << "Can't return expression of type " << expr->targetType
       << " from method with return type " << "umm"; // TODO ast::Type toString()
    error(*expr, ss.str());
  }
}
