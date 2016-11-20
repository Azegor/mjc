#include "semantic_visitor.hpp"

ast::DummySystem SemanticVisitor::dummySystem;
ast::DummySystemOut SemanticVisitor::dummySystemOut;

void SemanticVisitor::visitProgram(ast::Program &program) {
  currentProgram = &program;
  checkForDuplicates(program.getClasses());
  for (auto &cls : program.getClasses()) {
    cls->sortMembers();
  }
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
  currentMethod = &mm;
  mm.acceptChildren(this);
  currentMethod = nullptr;

  if (mm.getName() == "main") {
    this->mainMethodFound = true;
  }
  // INFO: check for >1 "main"-methods already done in visitMainMethodList
}

void SemanticVisitor::visitRegularMethod(ast::RegularMethod &method) {
  symTbl.enterScope(); // for parameters
  currentMethod = &method;
  method.acceptChildren(this);
  symTbl.leaveScope();
  currentMethod = nullptr;

  if (!method.getReturnType()->getSemaType().isVoid() &&
      method.getBlock()->cfb != sem::ControlFlowBehavior::Return) {
    error(method, "Non-Void method must return a value on every path", true);
  }
}

void SemanticVisitor::visitBlock(ast::Block &block) {
  // TODO: this currently adds an extra block for the outmost block,
  // the parameters therefore being alone in the outpust block.
  // since shadowing is forbidden this should be no problem
  symTbl.enterScope();
  block.acceptChildren(this);
  symTbl.leaveScope();

  for (auto stmt : block.getStatements()) {
    if (stmt->cfb == sem::ControlFlowBehavior::Return) {
      block.cfb = sem::ControlFlowBehavior::Return;
      return; // no more checking necessary, remaining code is dead
    }
  }
  block.cfb = sem::ControlFlowBehavior::MayContinue;
}

void SemanticVisitor::visitVariableDeclaration(ast::VariableDeclaration &decl) {
  decl.acceptChildren(this);

  if (decl.getInitializer() != nullptr &&
      !(decl.getType()->getSemaType() >= decl.getInitializer()->targetType)) {
    std::stringstream ss;
    ss << "Can't assign from " << decl.getInitializer()->targetType << " to "
       << decl.getType()->getSemaType();
    error(decl, ss.str());
  }

  // Filter out void types
  if (auto t = dynamic_cast<ast::PrimitiveType *>(decl.getType())) {
    if (t->getPrimType() == ast::PrimitiveType::PrimType::Void) {
      error(decl, "'void' is not a valid type in this context");
    }
  } else if (auto t = dynamic_cast<ast::ArrayType *>(decl.getType())) {
    // void[] types are not allowed either
    auto elementType = t->getElementType();
    if (auto p = dynamic_cast<ast::PrimitiveType *>(elementType)) {
      if (p->getPrimType() == ast::PrimitiveType::PrimType::Void) {
        error(decl, "'void' is not a valid type in this context");
      }
    }
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
}

void SemanticVisitor::visitVarRef(ast::VarRef &varRef) {
  varRef.acceptChildren(this);
  auto *def = symTbl.lookup(varRef.getSymbol());
  if (!def) {
    if (varRef.getName() == "System") {
      def = &dummySystem;
    } else {
      error(varRef, "Unknown variable '" + varRef.getSymbol().name + "'");
    }
  } else {
    varRef.targetType = def->getType()->getSemaType();
  }
  varRef.setDef(def);
}

void SemanticVisitor::visitParameter(ast::Parameter &param) {
  param.acceptChildren(this);
  auto &sym = param.getSymbol();
  if (symTbl.isDefinedInCurrentScope(sym)) {
    error(param, "Parameter '" + param.getSymbol().name + "' already defined");
  }
  symTbl.insert(sym, &param);
}

void SemanticVisitor::visitNewObjectExpression(ast::NewObjectExpression &expr) {
  expr.acceptChildren(this);
  auto *def = findClassByName(expr.getName());
  if (!def) {
    error(expr, "Undefined class '" + expr.getName() + "'");
  }
  expr.setDef(def);

  expr.targetType.setClass(expr.getName());
}

void SemanticVisitor::visitClassType(ast::ClassType &type) {
  type.acceptChildren(this);
  auto *def = findClassByName(type.getName());
  if (!def) {
    error(type, "Undefined class '" + type.getName() + "'");
  }
  type.setDef(def);
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
  if (currentClass == nullptr) {
    error(lit, "'this' can't be used outside of classes");
  }
  if (dynamic_cast<ast::RegularMethod *>(currentMethod)) {
    lit.targetType.setClass(this->currentClass->getName());
  } else {
    error(lit, "Cannot access class members in static method");
  }
}

void SemanticVisitor::visitBinaryExpression(ast::BinaryExpression &expr) {
  expr.acceptChildren(this);

  auto left = expr.getLeft();
  auto right = expr.getRight();

  switch (expr.getOperation()) {
  case ast::BinaryExpression::Op::Plus:
  case ast::BinaryExpression::Op::Minus:
  case ast::BinaryExpression::Op::Mul:
  case ast::BinaryExpression::Op::Div:
  case ast::BinaryExpression::Op::Mod:
    if (!right->targetType.isInt() || !left->targetType.isInt()) {
      error(expr, "operands must both be int");
    }
    expr.targetType.setInt();
    break;

  case ast::BinaryExpression::Op::Less:
  case ast::BinaryExpression::Op::LessEquals:
  case ast::BinaryExpression::Op::Greater:
  case ast::BinaryExpression::Op::GreaterEquals:
    if (!right->targetType.isInt() || !left->targetType.isInt()) {
      error(expr, "operands must both be int");
    }
    expr.targetType.setBool();
    break;

  case ast::BinaryExpression::Op::Or:
  case ast::BinaryExpression::Op::And:
    if (!right->targetType.isBool() || !left->targetType.isBool()) {
      error(expr, "operands must both be bool");
    }
    expr.targetType.setBool();
    break;

  case ast::BinaryExpression::Op::Assign:
    if (!(left->targetType >= right->targetType)) {
      std::stringstream ss;
      ss << "Can't assign value of type " << right->targetType
         << " to variable of type " << left->targetType;
      error(expr, ss.str());
    }
    expr.targetType = left->targetType;
    break;

  case ast::BinaryExpression::Op::Equals:
  case ast::BinaryExpression::Op::NotEquals:
    if (left->targetType >= right->targetType ||
        right->targetType >= left->targetType) {
      expr.targetType.setBool();
    } else {
      std::stringstream ss;
      ss << "Can't compare expression of type '" << left->targetType
         << "' to expression of type '" << right->targetType << "'";
      error(expr, ss.str());
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

  expr.targetType = expr.getArrayType()->getSemaType();
  std::cerr << expr.targetType << std::endl;
}

void SemanticVisitor::visitFieldAccess(ast::FieldAccess &access) {
  access.acceptChildren(this);

  // special handling for System.out.println()
  if (auto ref = dynamic_cast<ast::VarRef *>(access.getLeft())) {
    if (ref->getDef() == &dummySystem) {
      if (access.getName() == "out") {
        access.setDef(&dummySystemOut);
        return;
      } else {
        error(access,
              "invalid field access '" + access.getName() + "' on 'System'");
      }
    }
  }
  if (auto left = dynamic_cast<ast::FieldAccess *>(access.getLeft())) {
    if (left->getDef() == &dummySystemOut) {
      error(access, "invalid access '" + access.getName() + "' on System.out");
    }
  }

  auto &lhsType = access.getLeft()->targetType;
  if (!lhsType.isClass()) {
    error(access, "Left hand side of field access must be class type object");
  }

  auto cls = findClassByName(lhsType.name);
  assert(cls);
  auto &fieldName = access.getName();
  auto field = findFieldInClass(cls, fieldName);
  if (!field) {
    error(access,
          "Unknown field " + access.getName() + " in class " + fieldName);
  }
  access.setDef(field);
  access.targetType = field->getType()->getSemaType();
}

void SemanticVisitor::visitMethodInvocation(ast::MethodInvocation &invocation) {
  invocation.acceptChildren(this);

  // special handling for System.out.println()
  if (auto left = dynamic_cast<ast::VarRef *>(invocation.getLeft())) {
    auto *def = left->getDef();
    if (def == &dummySystem) {
      error(invocation,
            "invalid method call '" + invocation.getName() + "' on 'System'");
    }
    auto defType = def->getType();
    if (auto cl = dynamic_cast<ast::ClassType *>(defType)) {
      auto *classDef = findClassByName(cl->getName());
      assert(classDef != nullptr);

      ast::RegularMethod *method =
          findMethodInClass(classDef, invocation.getName());
      if (method == nullptr) {
        error(invocation, "Class " + cl->getName() +
                              " does not contain a method " +
                              invocation.getName());
      }
      // Valid method, valid class, propagate type!
      invocation.targetType = method->getReturnType()->getSemaType();
      invocation.setDef(method);
    } else {
      error(invocation, "Methods can only be invoked on class types");
    }
  } else if (auto left =
                 dynamic_cast<ast::FieldAccess *>(invocation.getLeft())) {
    // System.out.println special case: VarRef(System) - FieldAccess(out) -
    // MethodInvocation(println)
    if (left->getDef() == &dummySystemOut) {
      if (invocation.getName() == "println") {
        auto args = invocation.getArguments();
        if (args.size() != 1) {
          error(invocation,
                "System.out.println takes exactly one int argument, " +
                    std::to_string(args.size()) + " given");
        } else if (!args.at(0)->targetType.isInt()) {
          std::stringstream ss;
          ss << "System.out.println takes one int argument, "
                "but argument is of type "
             << args.at(0)->targetType;
          error(invocation, ss.str());
        }
        // Yep, System.out.println call
        invocation.targetType.setVoid();
        return;
      } else {
        error(invocation, "Invalid method call '" + invocation.getName() +
                              "' on 'System.out'");
      }
    }
  }
  auto left = invocation.getLeft();
  if (!left->targetType.isClass()) {
    error(invocation, "Methods can only be called on class types");
  }
  auto className = left->targetType.name;
  auto classDef = findClassByName(className);
  ast::RegularMethod *method =
      findMethodInClass(classDef, invocation.getName());
  if (method == nullptr) {
    error(invocation, "Class " + currentClass->getName() +
                          " does not have a method called '" +
                          invocation.getName() + "'");
  }
  const auto &methodParams = method->getParameters();
  const auto &invocArgs = invocation.getArguments();
  if (methodParams.size() != invocArgs.size()) {
    std::stringstream msg;
    msg << "invalid number of arguments: expected " << methodParams.size()
        << " but have " << invocArgs.size();
    error(invocation, msg.str());
  }
  for (size_t i = 0; i < methodParams.size(); ++i) {
    if (!(methodParams[i]->getType()->getSemaType() >=
          invocArgs[i]->targetType)) {
      std::stringstream msg;
      msg << "Invalid Argument type. Expected "
          << methodParams[i]->getType()->getSemaType() << " but have "
          << invocArgs[i]->targetType;
      error(*invocArgs[i], msg.str());
    }
  }

  invocation.setDef(method);
  invocation.targetType = method->getReturnType()->getSemaType();
}

void SemanticVisitor::visitIfStatement(ast::IfStatement &ifStatement) {
  ifStatement.acceptChildren(this);

  if (!ifStatement.getCondition()->targetType.isBool()) {
    std::stringstream ss;
    ss << "if condition must be boolean, not "
       << ifStatement.getCondition()->targetType;
    error(*ifStatement.getCondition(), ss.str());
  }

  // both then and else can be null, if they contain a single empty
  // statement
  auto thenCFB = ifStatement.getThenStatement()
                     ? ifStatement.getThenStatement()->cfb
                     : sem::ControlFlowBehavior::MayContinue;
  auto elseCFB = ifStatement.getElseStatement()
                     ? ifStatement.getElseStatement()->cfb
                     : sem::ControlFlowBehavior::MayContinue;
  ifStatement.cfb = sem::combineCFB(thenCFB, elseCFB);
}

void SemanticVisitor::visitWhileStatement(ast::WhileStatement &stmt) {
  stmt.acceptChildren(this);

  if (!stmt.getCondition()->targetType.isBool()) {
    std::stringstream ss;
    ss << "while condition must be boolean, not "
       << stmt.getCondition()->targetType;
    error(*stmt.getCondition(), ss.str());
  }

  // condition might be false -> ignore content of while block
  stmt.cfb = sem::ControlFlowBehavior::MayContinue;
}

void SemanticVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  stmt.acceptChildren(this);

  stmt.cfb = sem::ControlFlowBehavior::Return;

  if (currentMethod == nullptr) {
    error(stmt, "Return statement outside of Method");
  }

  auto methodReturnType = this->currentMethod->getReturnType()->getSemaType();

  auto expr = stmt.getExpression();
  if (expr == nullptr) {
    if (methodReturnType.isVoid()) {
      return;
    } else {
      error(stmt, "Non-Void methods must return a value");
    }
  }

  if (!(expr->targetType >= methodReturnType)) {
    std::stringstream ss;
    ss << "Can't return expression of type '" << expr->targetType
       << "' from method with return type '" << methodReturnType << "'";
    error(*expr, ss.str());
  }
}

void SemanticVisitor::visitUnaryExpression(ast::UnaryExpression &expr) {
  expr.acceptChildren(this);

  auto inner = expr.getExpression();

  if (expr.getOperation() == ast::UnaryExpression::Op::Neg) {
    if (!inner->targetType.isInt()) {
      std::stringstream ss;
      ss << "Negation requires int expression, not " << inner->targetType;
      error(*inner, ss.str());
    }
    expr.targetType.setInt();
  } else if (expr.getOperation() == ast::UnaryExpression::Op::Not) {
    if (!inner->targetType.isBool()) {
      std::stringstream ss;
      ss << "Negation requires boolean expression, not " << inner->targetType;
      error(*inner, ss.str());
    }
    expr.targetType.setBool();
  } else {
    assert(false);
  }
}

void SemanticVisitor::visitArrayAccess(ast::ArrayAccess &access) {
  access.acceptChildren(this);

  if (!access.getIndex()->targetType.isInt()) {
    error(*access.getIndex(), "Array indices must be integer expressions");
  }

  if (!access.getArray()->targetType.isArray()) {
    error(*access.getArray(), "Array access on non-array expression");
  }

  // access.targetType is array type -> decrease dimension by one
  access.targetType = access.getArray()->targetType;
  access.targetType.dimension -= 1;
  if (access.targetType.dimension == 0) {
    // array no more -> inner type surfaces
    access.targetType.kind = access.targetType.innerKind;
  }
}

void SemanticVisitor::visitField(ast::Field &field) {
  field.acceptChildren(this);

  if (auto t = dynamic_cast<ast::PrimitiveType *>(field.getType())) {
    if (t->getPrimType() == ast::PrimitiveType::PrimType::Void) {
      error(*t, "'void' is not a valid type in this context");
    }
  }
}
