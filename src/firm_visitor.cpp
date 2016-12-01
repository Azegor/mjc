#include "firm_visitor.hpp"

#ifndef LIBSEARCHDIR
#define LIBSEARCHDIR "."
#endif

__attribute__((unused))
static const char * get_node_mode(ir_node *node) {
  return get_mode_name(get_irn_mode(node));
}

// generates "boolean" (mode_Bu) 1/0 from a Cond node
// creates a new Block and returns the Phi
static ir_node* condToBoolean(ir_node* cond) {
  ir_node *projFalse = new_Proj(cond, mode_X, pn_Cond_false);
  ir_node *projTrue = new_Proj(cond, mode_X, pn_Cond_true);

  ir_node* const blockIn[] = { projFalse, projTrue };
  ir_node *exitBlock = new_Block(2, blockIn);
  set_cur_block(exitBlock);
  ir_node *falseNode = new_Const_long(mode_Bu, 0);
  ir_node *trueNode = new_Const_long(mode_Bu, 1);
  ir_node* const PhiIn[] = { falseNode, trueNode };
  return new_Phi(2, PhiIn, mode_Bu);
}

FirmVisitor::FirmVisitor(bool print, bool verify, bool gen) {
  ir_init();

  // disable libfirm optimizations
  // they can fuck things up by changing things we just added, e.g.
  //     node = new_Cmp
  //     is_Cmp(node) -> 0
  // jFirm does this as well
  set_optimize(0);

  //64 bit pointer mode
  auto _mode_P = new_reference_mode("P64", irma_twos_complement, 64, 64);
  set_modeP(_mode_P);

  this->printGraphs = print;
  this->verifyGraphs = verify;
  this->generateCode = gen;

  intType = new_type_primitive(mode_Is);
  boolType = new_type_primitive(mode_Bu);

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
    createClassEntity(*klass); // Will not create methods
  }
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

  if (generateCode) {
    FILE *f = fopen("test.s", "w");
    // XXX This only "works" on 64bit cpus
    be_parse_arg("isa=amd64");
    be_main(f, "test.java");
    fclose(f);
    int res = 0;
//     res |= system("gcc -c ../src/runtime.c -o runtime.o");
//     res |= system("ar rcs libruntime.a runtime.o");
    res |= system("gcc -static test.s -o _test_ -L" LIBSEARCHDIR " -lruntime");
    if (res) {
      throw std::runtime_error("Error while linking binary");
    }
  }
}

void FirmVisitor::visitMainMethod(ast::MainMethod &method) {
  ir_type *mainMethodType =
      new_type_method(0, 1, false, cc_cdecl_set, mtp_no_property);
  set_method_res_type(mainMethodType, 0, intType);

  ir_entity *entity =
      new_entity(get_glob_type(), new_id_from_str("main"), mainMethodType);

  auto &localVars = method.getVarDecls();
  ir_graph *mainMethodGraph = new_ir_graph(entity, localVars.size());
  set_current_ir_graph(mainMethodGraph);

  this->methods.insert({&method, FirmMethod(entity, 0,
        nullptr, localVars, mainMethodGraph)});
  this->currentMethod = &method;
  method.acceptChildren(this);

  ir_node *results[] = {new_Const_long(mode_Is, 0)};
  ir_node *returnNode  = new_Return(get_store(), 1, results);
  add_immBlock_pred(get_irg_end_block(mainMethodGraph), returnNode);

  // seals current block (-> all predecessors known)
  mature_immBlock(get_r_cur_block(mainMethodGraph));
  irg_finalize_cons(mainMethodGraph);
  dump_ir_graph(current_ir_graph, "");
  lower_highlevel_graph(mainMethodGraph);

  if (verifyGraphs) {
    if (irg_verify(mainMethodGraph) == 0)
      this->graphErrors++;
  }
}

void FirmVisitor::visitRegularMethod(ast::RegularMethod &method) {
  // Types in this->methods have already been created in visitClass!
  auto firmMethod = &this->methods.at(&method);
  auto methodGraph = firmMethod->graph;
  this->currentMethod = &method;

  set_current_ir_graph(methodGraph);
  method.acceptChildren(this);

  // If the return value is void, insert an empty return node
  if (method.getReturnType()->getSemaType().isVoid()) {
    ir_node *returnNode = new_Return(get_store(), 0, nullptr);
    add_immBlock_pred(get_irg_end_block(methodGraph), returnNode);
  }

  // "... mature the current block, which means fixing the number of their predecessors"
  mature_immBlock(get_r_cur_block(methodGraph));

  irg_finalize_cons(methodGraph);
  dump_ir_graph(current_ir_graph, "");
  lower_highlevel_graph(methodGraph);

  if (verifyGraphs) {
    if (irg_verify(methodGraph) == 0)
      this->graphErrors++;
  }
}

void FirmVisitor::createClassEntity(ast::Class &klass) {
  // create separately since methods may use other classes als return or parameter type
  ir_type *classType = new_type_class(klass.getName().c_str());
  ir_entity *classEntity = new_entity(get_glob_type(), klass.getName().c_str(), classType);
  this->classes.insert({&klass, FirmClass(classEntity)});
}

void FirmVisitor::visitClass(ast::Class &klass) {
  // We do not recurse into children here since that will be done separately
  // in visitProgram(). Instead, collect the types of all methods and to the same there.

  ir_type *classType = get_entity_type(classes.at(&klass).entity);

  auto &methods = klass.getMethods()->getMethods();
  for (auto &method : methods) {
    int numReturnValues = method->getReturnType()->getSemaType().isVoid() ? 0 : 1;
    auto &parameters = method->getParameters();
    ir_type *methodType = new_type_method(parameters.size() + 1, numReturnValues,
                                          false, cc_cdecl_set, mtp_no_property);

    if (numReturnValues > 0) {
      set_method_res_type(methodType, 0, this->getIrType(method->getReturnType()));
    }
    // new pointer-type pointing to existing type (classType)
    ir_type *thisParamType = new_type_pointer(classType);
    set_method_param_type(methodType, 0, thisParamType);

    int numParams = 1; // includes explicit 'this' parameter
    for (auto &param : parameters) {
      set_method_param_type(methodType, numParams, getIrType(param->getType()));
      numParams++;
    }

    ir_entity *methodEntity = new_entity(classType,
                                   new_id_from_str(method->getMangledName().c_str()),
                                   methodType);

    auto &localVars = method->getVarDecls();
    /* "returns a new graph consisting of a start block, a regular block
     * and an end block" */
    ir_graph *methodGraph = new_ir_graph(methodEntity,
           numParams + localVars.size()); // number of local variables including parameters
    set_current_ir_graph(methodGraph);

    // Add projections for arguments
    ir_node *lastBlock = get_r_cur_block(methodGraph);
    // set the start block to be the current block
    set_r_cur_block(methodGraph, get_irg_start_block(methodGraph));
    ir_node *args = get_irg_args(methodGraph);

    ir_node **paramNodes = new ir_node*[numParams];
    paramNodes[0] = new_Proj(args, mode_P, 0);
    int i = 1;
    for(auto &param : parameters) {
      paramNodes[i] = new_Proj(args, getIrMode(param->getType()), i);
      set_value(i, paramNodes[i]); // TODO necessary?
      i++;
    }

    set_r_cur_block(methodGraph, lastBlock);
    this->methods.insert({method.get(), FirmMethod(methodEntity, (size_t)numParams,
          paramNodes, localVars, methodGraph)});
  }

  auto &fields = klass.getFields()->getFields();
  auto firmClass = &classes.at(&klass);
  int offset = 0;
  for(auto &field : fields) {
    ir_type *fieldType = getIrType(field->getType());
    ir_entity *ent = new_entity(classType,
                                field->getName().c_str(),
                                fieldType);
    set_entity_offset(ent, offset);
    firmClass->fieldEntities.push_back(FirmField{field.get(), ent});
    offset += get_type_size(fieldType);
  }

  set_type_state(classType, layout_fixed);
  set_type_size(classType, offset);
}

void FirmVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  ir_node *end = get_irg_end_block(current_ir_graph);
  ir_node *ret;

  if (stmt.getExpression() != nullptr) {
    stmt.acceptChildren(this);

    ir_node *results[] = {popNode()->load()};
    ret = new_Return(get_store(), 1, results);
  } else {
    if (dynamic_cast<ast::MainMethod *>(currentMethod)) {
      ir_node *results[] = {new_Const_long(mode_Is, 0)};
      ret = new_Return(get_store(), 1, results);
    }
    ret = new_Return(get_store(), 0, nullptr);
  }

  add_immBlock_pred(end, ret);
}

void FirmVisitor::visitMethodInvocation(ast::MethodInvocation &invocation) {
  if (invocation.isSysoutCall()) {
    // "to create the call we first create a node representing the address
    //  of the function we want to call" ... "then we use new_Call to
    //  create the call"
    invocation.acceptChildren(this);

    ir_node *args[] = {popNode()->load()}; // int argument
    ir_node *store = get_store();
    ir_node *callee = new_Address(sysoutEntity);
    ir_node *callNode = new_Call(store, callee, 1, args, this->sysoutType);

    // Update the current store
    ir_node *newStore = new_Proj(callNode, get_modeM(), pn_Call_M);
    set_store(newStore);
    pushNode(nullptr); // needs to return a node for consistency!
  } else {
    auto left = invocation.getLeft();
    // This should be true now after semantic analysis
    assert(left->targetType.isClass());
    auto leftClass = currentProgram->findClassByName(left->targetType.name);
    auto leftFirmClass = this->classes.at(leftClass);

    auto firmMethod = &this->methods.at(invocation.getDef());

    size_t nArgs = 1 + invocation.getArguments().size();
    // TODO: Handle arguments
    std::vector<ir_node *> args;
    args.resize(nArgs);
    left->accept(this);
    args[0] = popNode()->load();
    int i = 1;
    for (auto &arg : invocation.getArguments()) {
      arg->accept(this);
      args[i] = popNode()->load();
      ++i;
    }
    ir_node *store = get_store();
    ir_node *callee = new_Address(firmMethod->entity);
    ir_node *callNode =
        new_Call(store, callee, nArgs, args.data(), firmMethod->type());

    // Update the current store
    ir_node *newStore = new_Proj(callNode, get_modeM(), pn_Call_M);
    set_store(newStore);

    // get the result
    if (!invocation.targetType.isVoid()) {
      ir_node *tuple = new_Proj(callNode, mode_T, pn_Call_T_result);
      ir_node *result = new_Proj(tuple, getIrMode(invocation.targetType), 0);
      pushNode(result);
    } else {
      pushNode(nullptr); // needs to return a node for consistency!
    }
  }
}

void FirmVisitor::visitIntLiteral(ast::IntLiteral &lit) {
  // We only have signed 32 bit integers.
  ir_node *node = new_Const_long(mode_Is, lit.getValue());
  pushNode(node);
}

void FirmVisitor::visitBoolLiteral(ast::BoolLiteral &lit) {
  ir_node *node = new_Const_long(mode_Bu, lit.getValue() ? 1 : 0);
  pushNode(node);
}

void FirmVisitor::visitThisLiteral(ast::ThisLiteral &lit) {
  (void)lit;
  // we always have an explicit `this` parameter at position 0
  auto firmMethod = &this->methods.at(currentMethod);
  pushNode(firmMethod->params[0]);
//   pushNode(std::make_unique<VarValue>(0, classes.at(currentClass).type()));
}

void FirmVisitor::visitNullLiteral(ast::NullLiteral &lit) {
  (void)lit;
  pushNode(new_Const(new_tarval_from_long(0, mode_P)));
}

void FirmVisitor::visitBinaryExpression(ast::BinaryExpression &expr) {
  expr.getLeft()->accept(this);
  auto leftNode = popNode();
  expr.getRight()->accept(this);
  auto rightNode = popNode();
  ir_node* outNode = nullptr;
  bool want_conversion = false;

  switch (expr.getOperation()) {
  case ast::BinaryExpression::Op::Assign: {
    auto rightVal = rightNode->load();
    leftNode->store(rightVal);
    pushNode(rightVal); // might be used further TODO: is this correct so?
    break;
//     auto firmMethod = &methods.at(this->currentMethod);
//     if (auto varRef = dynamic_cast<ast::VarRef *>(expr.getLeft())) {
//       auto def = varRef->getDef();
//       if (auto locVar = dynamic_cast<ast::VariableDeclaration *>(def)) {
//         auto pos = firmMethod->nParams + locVar->getIndex();
//         set_value(pos, rightNode);
//       } else if (auto fieldDef = dynamic_cast<ast::Field *>(def)) {
//         (void)fieldDef;
//         assert(false);
//       } else {
//         assert(false);
//       }
//     } else {
//       assert(false);
//     }
  }
  case ast::BinaryExpression::Op::Or:
    assert(false);
    break;
  case ast::BinaryExpression::Op::And:
    assert(false);
    break;
  case ast::BinaryExpression::Op::Equals:
    outNode = new_Cond(new_Cmp(leftNode->load(), rightNode->load(), ir_relation_equal));
    want_conversion = true;
    break;
  case ast::BinaryExpression::Op::NotEquals:
    outNode = new_Cond(new_Cmp(leftNode->load(), rightNode->load(), ir_relation_less_greater));
    want_conversion = true;
    break;
  case ast::BinaryExpression::Op::Less:
    outNode = new_Cond(new_Cmp(leftNode->load(), rightNode->load(), ir_relation_less));
    want_conversion = true;
    break;
  case ast::BinaryExpression::Op::LessEquals:
    outNode = new_Cond(new_Cmp(leftNode->load(), rightNode->load(), ir_relation_less_equal));
    want_conversion = true;
    break;
  case ast::BinaryExpression::Op::Greater:
    outNode = new_Cond(new_Cmp(leftNode->load(), rightNode->load(), ir_relation_greater));
    want_conversion = true;
    break;
  case ast::BinaryExpression::Op::GreaterEquals:
    outNode = new_Cond(new_Cmp(leftNode->load(), rightNode->load(), ir_relation_greater_equal));
    want_conversion = true;
    break;
  case ast::BinaryExpression::Op::Plus:
    outNode = new_Add(leftNode->load(), rightNode->load());
    break;
  case ast::BinaryExpression::Op::Minus:
    outNode = new_Sub(leftNode->load(), rightNode->load());
    break;
  case ast::BinaryExpression::Op::Mul:
    outNode = new_Mul(leftNode->load(), rightNode->load());
    break;
  case ast::BinaryExpression::Op::Div: {
    ir_node *memory = get_store();
    ir_node *pin = new_Pin(memory);
    set_store(pin);
    ir_node *divNode = new_DivRL(pin, leftNode->load(), rightNode->load(), op_pin_state_pinned);
    outNode = new_Proj(divNode, mode_Is, pn_Div_res);
    break;
  }
  case ast::BinaryExpression::Op::Mod: {
    ir_node *memory = get_store();
    ir_node *pin = new_Pin(memory);
    set_store(pin);
    ir_node *modNode = new_Mod(pin, leftNode->load(), rightNode->load(), op_pin_state_pinned);
    outNode = new_Proj(modNode, mode_Is, pn_Mod_res);
    break;
  }
  default:
    assert(false);
    break;
  }

  if(outNode) {
    if(want_conversion && ! control_flow_from_expr) {
      outNode = condToBoolean(outNode);
    }
    control_flow_from_expr = false;
    pushNode(outNode);
  }
}

void FirmVisitor::visitVarRef(ast::VarRef &ref) {
  if (ref.getDef() == &ast::VarRef::dummySystem) {
    // XXX Have to add a dummy node here?
    return;
  }

  if (auto param = dynamic_cast<ast::Parameter*>(ref.getDef())) {
    // Param refs are only possible in regular methods
//     auto method = dynamic_cast<ast::RegularMethod*>(this->currentMethod);
    size_t paramIndex = param->getIndex();
    assert(paramIndex < this->methods.at(this->currentMethod).nParams);
    pushNode(std::make_unique<VarValue>(paramIndex, getIrMode(param->getType())));
  } else if (auto decl = dynamic_cast<ast::VariableDeclaration*>(ref.getDef())) {
    auto firmMethod = &methods.at(this->currentMethod);
    size_t varIndex = firmMethod->nParams; // first parameters, then local vars
    varIndex += decl->getIndex();
    //     ir_node *val = get_r_value (current_ir_graph, varIndex,
//                                 getIrMode(decl->getType()));
    pushNode(std::make_unique<VarValue>(varIndex, getIrMode(decl->getType())));
  } else if (auto field = dynamic_cast<ast::Field*>(ref.getDef())) {
    auto firmClass = &classes.at(this->currentClass);
    for (auto &fieldEnt : firmClass->fieldEntities) {
      if (fieldEnt.field == field) {
        auto firmMethod = &this->methods.at(this->currentMethod);
        ir_node *thisPointer = firmMethod->params[0];
        ir_node *member = new_Member(thisPointer, fieldEnt.entity);
        assert(is_Member(member));
        pushNode(std::make_unique<FieldValue>(member));
        return;
      }
    }
    assert(false);
  } else {
    assert(false);
  }
}

void FirmVisitor::visitUnaryExpression(ast::UnaryExpression &expr) {
  expr.acceptChildren(this);

  switch(expr.getOperation()) {
  case ast::UnaryExpression::Op::Not: {
    ir_node *zero = new_Const_long(mode_Bu, 0);
    ir_node *outNode = new_Cond(new_Cmp(popNode()->load(), zero, ir_relation_equal));
    if (! control_flow_from_expr) {
      outNode = condToBoolean(outNode);
    }
    control_flow_from_expr = false;
    pushNode(outNode);
    break; }
  case ast::UnaryExpression::Op::Neg:
    pushNode(new_Minus(popNode()->load()));
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
    pos += decl.getIndex();

    set_r_value(current_ir_graph, pos, popNode()->load());
  }
}

void FirmVisitor::visitField(ast::Field &field) {
  (void) field;
}

void FirmVisitor::visitFieldAccess(ast::FieldAccess &access) {
  // sysout special case...
  if (access.getDef() == &ast::FieldAccess::dummySystemOut) {
    return;
  }
  auto firmClass = &classes.at(
      currentProgram->findClassByName(access.getLeft()->targetType.name));
  // Left is never null!
  access.getLeft()->accept(this);
  ir_node *leftNode = popNode()->load();
  assert(get_irn_mode(leftNode) == mode_P);

  ir_entity *rightEntity = nullptr;
  for(auto &fieldEntity : firmClass->fieldEntities)
    if (fieldEntity.field == access.getDef()) {
      rightEntity = fieldEntity.entity;
      break;
    }
  assert(rightEntity != nullptr);


  ir_node *member = new_Member(leftNode, rightEntity);

  pushNode(std::make_unique<FieldValue>(member));
}

void FirmVisitor::visitNewObjectExpression(ast::NewObjectExpression &expr) {
  auto thisClass = &classes.at(expr.getDef());
  ir_type *classType = thisClass->type();
  ir_type *classPointerType = new_type_pointer(classType);
  ir_entity *callocEntity = makeCalloc(classPointerType);

  ir_node *args[2] = {new_Const_long(mode_Is, 1),
                      new_Size(mode_Is, thisClass->type())};
  ir_node *store = get_store();
  ir_node *callee = new_Address(callocEntity);
  ir_node *callNode = new_Call(store, callee, 2, args, get_entity_type(callocEntity));
  ir_node *newStore = new_Proj(callNode, mode_M, pn_Call_M);
  set_store(newStore);

  ir_node *tuple  = new_Proj(callNode, mode_T, pn_Call_T_result);
  ir_node *result = new_Proj(tuple, mode_P, 0);

  pushNode(result);
}

void FirmVisitor::visitArrayAccess(ast::ArrayAccess& arrayAccess)
{
  arrayAccess.getArray()->accept(this);
  ir_node *arrayNode = popNode()->load();
  arrayAccess.getIndex()->accept(this);
  ir_node *indexNode = popNode()->load();
  size_t elemSize = arrayAccess.getArray()->targetType.getArrayInnerType().calculateSize();
  ir_node *addr = new_Add(arrayNode, new_Mul(new_Conv(indexNode, mode_Ls), new_Const_long(mode_Ls, elemSize)));

  auto elementType =
      getIrType(arrayAccess.getArray()->targetType.getArrayInnerType());
  pushNode(std::make_unique<ArrayValue>(addr, elementType));
}

void FirmVisitor::visitNewArrayExpression(ast::NewArrayExpression &expr) {
  ir_type *arrayType = getIrType(expr.getArrayType()); // correctly handles multiple dimensions
  ir_entity *callocEntity = makeCalloc(arrayType);

  expr.getSize()->accept(this);
  auto elementType = getIrType(expr.getArrayType()->getSemaType().getArrayInnerType());
  ir_node *args[2] = {popNode()->load(), new_Size(mode_Is, elementType)};
  ir_node *store = get_store();
  ir_node *callee = new_Address(callocEntity);
  ir_node *callNode = new_Call(store, callee, 2, args, get_entity_type(callocEntity));
  ir_node *newStore = new_Proj(callNode, mode_M, pn_Call_M);
  set_store(newStore);

  ir_node *tuple  = new_Proj(callNode, mode_T, pn_Call_T_result);
  ir_node *result = new_Proj(tuple, mode_P, 0);

  pushNode(result);
}

void FirmVisitor::visitExpressionStatement(ast::ExpressionStatement &exprStmt) {
  exprStmt.acceptChildren(this);
  popNode(); // discard any values form the expression
}

void FirmVisitor::visitIfStatement(ast::IfStatement &stmt) {
  control_flow_from_expr = true;
  stmt.getCondition()->accept(this);
  ir_node *condNode = popNode()->load();

  ir_node *trueProj = new_Proj(condNode, mode_X, pn_Cond_true);
  ir_node *falseProj = new_Proj(condNode, mode_X, pn_Cond_false);

  ir_node *afterBlock = new_immBlock();

  if (stmt.getThenStatement() != nullptr) {
    ir_node *thenBlock = new_immBlock();
    add_immBlock_pred(thenBlock, trueProj);
    set_cur_block(thenBlock);

    stmt.getThenStatement()->accept(this);
    mature_immBlock(thenBlock);
    if (stmt.getThenStatement()->cfb == sem::ControlFlowBehavior::MayContinue) {
      add_immBlock_pred(afterBlock, new_Jmp());
    }
  } else {
    add_immBlock_pred(afterBlock, trueProj);
  }

  if (stmt.getElseStatement() != nullptr) {
    ir_node *elseBlock = new_immBlock();
    add_immBlock_pred(elseBlock, falseProj);
    set_cur_block(elseBlock);

    stmt.getElseStatement()->accept(this);
    mature_immBlock(elseBlock);
    if (stmt.getElseStatement()->cfb == sem::ControlFlowBehavior::MayContinue) {
      add_immBlock_pred(afterBlock, new_Jmp());
    }
  } else {
    add_immBlock_pred(afterBlock, falseProj);
  }

  mature_immBlock(afterBlock);
  set_cur_block(afterBlock);
}

void FirmVisitor::visitWhileStatement(ast::WhileStatement &stmt) {
  ir_node *whileBlock = new_immBlock();
  add_immBlock_pred(whileBlock, new_Jmp());
  set_cur_block(whileBlock);

  control_flow_from_expr = true;
  stmt.getCondition()->accept(this);
  ir_node *condNode = popNode()->load();

  ir_node *trueProj = new_Proj(condNode, mode_X, pn_Cond_true);
  ir_node *falseProj = new_Proj(condNode, mode_X, pn_Cond_false);

  ir_node *afterBlock = new_immBlock();
  add_immBlock_pred(afterBlock, falseProj);

  ir_node *loopBlock = new_immBlock();
  add_immBlock_pred(loopBlock, trueProj);
  set_cur_block(loopBlock);
  keep_alive(loopBlock);

  get_store();

  // not sure if this can happen?
  if (stmt.getStatement() != nullptr) {
    stmt.getStatement()->accept(this);
  }

  // don't add jump if the Statement returns
  if ((stmt.getStatement() == nullptr) ||
      (stmt.getStatement()->cfb == sem::ControlFlowBehavior::MayContinue)) {
    add_immBlock_pred(whileBlock, new_Jmp());
  }
  mature_immBlock(whileBlock);
  mature_immBlock(loopBlock);

  mature_immBlock(afterBlock);
  set_cur_block(afterBlock);
}

