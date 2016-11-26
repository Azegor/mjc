#include "firm_visitor.hpp"

FirmVisitor::FirmVisitor(bool print, bool verify) {
  ir_init();

  this->printGraphs = print;
  this->verifyGraphs = verify;

  intType = new_type_primitive(mode_Is);
  boolType = new_type_primitive(mode_Bu);
  arrayType = new_type_primitive(mode_P);

  // System.out.println takes just 1 param and returns void
  sysoutType = new_type_method(1, 0, false, cc_cdecl_set, mtp_no_property);
  set_method_param_type(sysoutType, 0, intType);
  sysoutEntity = new_global_entity(get_glob_type(), "print_int", sysoutType,
                                   ir_visibility_external, IR_LINKAGE_DEFAULT);
}

void FirmVisitor::visitProgram(ast::Program &program) {
  this->currentProgram = &program;
  auto &classes = program.getClasses();

  // First, collect all class types
  for (auto &klass : classes) {
    klass->accept(this); // Will not recurse into children
  }

  for (auto &klass : classes) {
    this->currentClass = klass.get();
    klass->acceptChildren(this);
  }

  assert(nodeStack.size() == 0);

  if (printGraphs) {
    dump_all_ir_graphs("");
  }
}

void FirmVisitor::visitMainMethod(ast::MainMethod &method) {
  ir_type *mainMethodType =
      new_type_method(0, 0, false, cc_cdecl_set, mtp_no_property);

  ir_entity *entity =
      new_entity(get_glob_type(), new_id_from_str("main"), mainMethodType);

  auto localVars = method.getBlock()->countVariableDeclarations();
  ir_graph *mainMethodGraph = new_ir_graph(entity, localVars.size());
  set_current_ir_graph(mainMethodGraph);

  this->methods.insert({&method, FirmMethod(mainMethodType, entity, 0,
        nullptr, localVars, mainMethodGraph)});
  this->currentMethod = &method;
  method.acceptChildren(this);

  // "... mature the current block, which means fixing the number of their predecessors"
  mature_immBlock(get_r_cur_block(mainMethodGraph));
  irg_finalize_cons(mainMethodGraph);

  if (verifyGraphs) {
    irg_verify(mainMethodGraph);
  }
}

void FirmVisitor::visitRegularMethod(ast::RegularMethod &method) {
  // Types in this->methods have already been created in visitClass!
  auto parameters = method.getParameters();
  auto firmMethod = &this->methods.at(&method);
  auto methodGraph = firmMethod->graph;
  this->currentMethod = &method;

  ir_node *args_node = get_irg_args(methodGraph);
  // Initialize all parameters/local variables
  set_r_value(methodGraph, 0, new_Proj(args_node, mode_P, 0));
  size_t i = 1;
  for (auto &param : parameters) {
    set_r_value(methodGraph, i, new_Proj(args_node, mode_Is, 0)); // TODO: Correct mode
    (void)param;
    i++;
  }

  set_current_ir_graph(methodGraph);
  method.acceptChildren(this);

  // "... mature the current block, which means fixing the number of their predecessors"
  mature_immBlock(get_r_cur_block(methodGraph));

  irg_finalize_cons(methodGraph);

  if (verifyGraphs) {
    irg_verify(methodGraph);
  }
}

void FirmVisitor::visitClass(ast::Class &klass) {
  ir_type *classType = new_type_class(klass.getName().c_str());
  ir_entity *classEntity = new_entity(get_glob_type(), klass.getName().c_str(), classType);
  this->classes.insert({&klass, FirmClass(classType, classEntity)});
  // We do not recurse into children here since that will be done separately
  // in visitProgram(). Instead, collect the types of all methods and to the same there.

  auto methods = klass.getMethods()->getMethods();
  for (auto &method : methods) {
    int numReturnValues = method->getReturnType()->getSemaType().isVoid() ? 0 : 1;
    auto parameters = method->getParameters();
    ir_type *methodType = new_type_method(parameters.size() + 1, numReturnValues,
                                          false, cc_cdecl_set, mtp_no_property);

    if (numReturnValues > 0)
      set_method_res_type(methodType, 0, intType);

    ir_type *thisParamType = new_type_pointer(classType);
    set_method_param_type(methodType, 0, thisParamType);

    int numParams = 1; // includes explicit 'this' parameter
    for (auto &param : parameters) {
      set_method_param_type(methodType, numParams, getIrType(param->getType()));
      numParams++;
    }

    ir_entity *entity = new_entity(classType,
                                   new_id_from_str(method->getName().c_str()),
                                   methodType);

    auto localVars = method->getBlock()->countVariableDeclarations();
    /* "returns a new graph consisting of a start block, a regular block
     * and an end block" */
    ir_graph *methodGraph = new_ir_graph(entity,
           numParams + localVars.size()); // number of local variables including parameters
    set_current_ir_graph(methodGraph);

    // Add projections for arguments
    ir_node *lastBlock = get_r_cur_block(methodGraph);
    // set the start block to be the current block
    set_r_cur_block(methodGraph, get_irg_start_block(methodGraph));
    ir_node *args = get_irg_args(methodGraph);

    ir_node **paramNodes = new ir_node*[numParams];
    paramNodes[0] = new_Proj(args, mode_Is, 0); // This parameter TODO: Correct mode
    int i = 1;
    for(auto &param : parameters) {
      (void)param;
      paramNodes[i] = new_Proj(args, mode_Is, i); // TODO: Correct mode
      i++;
    }

    set_r_cur_block(methodGraph, lastBlock);
    this->methods.insert({method, FirmMethod(methodType, entity, (size_t)numParams,
          paramNodes, localVars, methodGraph)});
  }
}

void FirmVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  (void)stmt;
  ir_graph *currentGraph = get_current_ir_graph();

  stmt.acceptChildren(this);

  ir_node *results[1] = {popNode()};

  ir_node *store = get_store();
  ir_node *ret = new_Return(store, 1, results);
  ir_node *end = get_irg_end_block(currentGraph);

  add_immBlock_pred(end, ret);
}

void FirmVisitor::visitMethodInvocation(ast::MethodInvocation &invocation) {
  if (invocation.isSysoutCall()) {
    // "to create the call we first create a node representing the address
    //  of the function we want to call" ... "then we use new_Call to
    //  create the call"
    invocation.acceptChildren(this);

    ir_node *args[1] = {popNode()};
    ir_node *store = get_store();
    ir_node *callee = new_Address(sysoutEntity);
    ir_node *callNode = new_Call(store, callee, 1, args, this->sysoutType);

    // Update the current store
    ir_node *newStore = new_Proj(callNode, get_modeM(), pn_Call_M);
    set_store(newStore);
  } else {
    auto left = invocation.getLeft();
    // This should be true now after semantic analysis
    assert(left->targetType.isClass());
    // TODO: Implement this for non-ThisLiteral left-sides
    assert(dynamic_cast<ast::ThisLiteral*>(left));
    auto leftClass = currentProgram->findClassByName(left->targetType.name);
    auto leftFirmClass = this->classes.at(leftClass);

    auto firmMethod = &this->methods.at(invocation.getDef());

    size_t nArgs = 1 + invocation.getArguments().size();
    // TODO: Handle arguments
    assert(nArgs == 1);
    ir_node **args = new ir_node*[nArgs];
    left->accept(this);
    args[0] = popNode();
    ir_node *store = get_store();
    ir_node *callee = new_Address(firmMethod->entity);
    ir_node *callNode = new_Call(store, callee, nArgs, args, firmMethod->type);

    // Update the current store
    ir_node *newStore = new_Proj(callNode, get_modeM(), pn_Call_M);
    set_store(newStore);

    // get the result
    if (!invocation.targetType.isVoid()) {
      ir_node *tuple = new_Proj(callNode, mode_T, pn_Call_T_result);
      ir_node *result = new_Proj(tuple, mode_Is, 0); // TODO: Correct type
      pushNode(result);
    }
  }
}

void FirmVisitor::visitIntLiteral(ast::IntLiteral &lit) {
  // We only have signed 32 bit integers.
  ir_node *node = new_Const(new_tarval_from_long(lit.getValue(), mode_Is));
  pushNode(node);
}

void FirmVisitor::visitBoolLiteral(ast::BoolLiteral &lit) {
  ir_node *node = new_Const(new_tarval_from_long(lit.getValue() ? 1 : 0, mode_Bu));
  pushNode(node);
}

void FirmVisitor::visitThisLiteral(ast::ThisLiteral &lit) {
  (void)lit;
  // we always have an explicit `this` parameter at position 0
  pushNode(get_r_value(current_ir_graph, 0, mode_P));
}

void FirmVisitor::visitNullLiteral(ast::NullLiteral &lit) {
  (void)lit;
  pushNode(new_Const(new_tarval_from_long(0, mode_P)));
}

void FirmVisitor::visitBinaryExpression(ast::BinaryExpression &expr) {
  expr.getLeft()->accept(this);
  ir_node *leftNode = popNode();
  expr.getRight()->accept(this);
  ir_node *rightNode = popNode();

  switch (expr.getOperation()) {
  case ast::BinaryExpression::Op::Assign:
    break;
  case ast::BinaryExpression::Op::Or:
    break;
  case ast::BinaryExpression::Op::And:
    break;
  case ast::BinaryExpression::Op::Equals:
    break;
  case ast::BinaryExpression::Op::NotEquals:
    break;
  case ast::BinaryExpression::Op::Less:
    break;
  case ast::BinaryExpression::Op::LessEquals:
    break;
  case ast::BinaryExpression::Op::Greater:
    break;
  case ast::BinaryExpression::Op::GreaterEquals:
    break;
  case ast::BinaryExpression::Op::Plus:
    pushNode(new_Add(leftNode, rightNode));
    break;
  case ast::BinaryExpression::Op::Minus:
    pushNode(new_Sub(leftNode, rightNode));
    break;
  case ast::BinaryExpression::Op::Mul:
    pushNode(new_Mul(leftNode, rightNode));
    break;
  case ast::BinaryExpression::Op::Div: {
    ir_node *store = get_store();
    // TODO: Wtf does the last parameter mean?
    pushNode(new_Div(store, leftNode, rightNode, 0));
    break;
  }
  case ast::BinaryExpression::Op::Mod: {
    ir_node *store = get_store();
    // TODO: Wtf does the last parameter mean?
    pushNode(new_Mod(store, leftNode, rightNode, 0));
    break;
  }
  default:
    assert(false);
    break;
  }
}

void FirmVisitor::visitVarRef(ast::VarRef &ref) {
  (void) ref;
  if (ref.getName() == "System") {
    // XXX Have to add a dummy node here?
    return;
  }

  if (auto param = dynamic_cast<ast::Parameter*>(ref.getDef())) {
    // Param refs are only possible in regular methods
    auto method = dynamic_cast<ast::RegularMethod*>(this->currentMethod);
    auto firmMethod = &this->methods.at(this->currentMethod);
    // Parameter names are unique at this point
    auto params = method->getParameters();
    size_t paramIndex = 0;
    for (auto &p : params) {
      if (p == param) {
        break;
      }
      paramIndex ++;
    }
    assert(paramIndex < firmMethod->nParams);
    // TODO: Proj of that parameter? or get_r_value?
    pushNode(firmMethod->params[1 + paramIndex]); // 1 because of the this parameter
  } else if (auto decl = dynamic_cast<ast::VariableDeclaration*>(ref.getDef())) {
    auto firmMethod = &methods.at(this->currentMethod);
    size_t pos = firmMethod->nParams; // first parameters, then local vars

    for (auto &lv : firmMethod->localVars) {
      if (lv == decl)
        break;
      pos ++;
    }

    // TODO: Use Proj here?
    ir_node *val = get_r_value (current_ir_graph, pos, mode_Is); // TODO: Correct mode
    pushNode(val);
  } else if (auto field = dynamic_cast<ast::Field*>(ref.getDef())) {
    auto firmClass = &classes.at(this->currentClass);
    bool found = false;
    for (auto &fieldEnt : firmClass->fieldEntities) {
      if (fieldEnt.field == field) {
        // XXX: Always creates pointer types?
        ir_node *member = new_Member(new_Address(firmClass->entity),
                                     fieldEnt.entity);
        pushNode(member);
        found = true;
        break;
      }
    }

    // Just to be sure
    assert(found);
  } else {
    assert(false);
  }
}

void FirmVisitor::visitUnaryExpression(ast::UnaryExpression &expr) {
  expr.acceptChildren(this);

  switch(expr.getOperation()) {
  case ast::UnaryExpression::Op::Not:
    pushNode(new_Not(popNode()));
    break;
  case ast::UnaryExpression::Op::Neg:
    pushNode(new_Minus(popNode()));
    break;
  default:
    assert(false);
  }
}

void FirmVisitor::visitVariableDeclaration(ast::VariableDeclaration &decl) {
  if (decl.getInitializer() != nullptr) {
    decl.getInitializer()->accept(this);
    auto firmMethod = &methods.at(this->currentMethod);
    size_t pos = firmMethod->nParams; // first parameters, then local vars

    for (auto &lv : firmMethod->localVars) {
      if (lv == &decl)
        break;
      pos ++;
    }

    set_r_value(current_ir_graph, pos, popNode());
  }
}

void FirmVisitor::visitField(ast::Field &field) {
  auto firmClass = &classes.at(this->currentClass);
  ir_type *fieldType = new_type_primitive(mode_Is); // TODO: Use correct type;
  ir_entity *ent = new_entity(firmClass->type,
                              field.getName().c_str(),
                              fieldType);
  firmClass->fieldEntities.push_back(FirmField{&field, ent});
}
