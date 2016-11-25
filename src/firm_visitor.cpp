#include "firm_visitor.hpp"

FirmVisitor::FirmVisitor() {
  ir_init();

  intType = new_type_primitive(mode_Is);
  boolType = new_type_primitive(mode_Bu);
  arrayType = new_type_primitive(mode_P);

  // System.out.println takes just 1 param and returns void
  sysoutType = new_type_method(1, 0, false, cc_cdecl_set, mtp_no_property);
  set_method_param_type(sysoutType, 0, intType);
  // TODO: printf() doesn't take an int?
  sysoutEntity = new_entity(get_glob_type(), "printf", sysoutType);
}

void FirmVisitor::visitProgram(ast::Program &program) {
  auto &classes = program.getClasses();

  // First, collect all class types
  for (auto &klass : classes) {
    ir_type *classType = new_type_class(klass->getName().c_str());
    this->classTypes[klass.get()] = classType;
  }

  for (auto &klass : classes) {
    this->currentClassType = classTypes[klass.get()];
    klass->acceptChildren(this);
  }

  dump_all_ir_graphs("");
}

void FirmVisitor::visitMainMethod(ast::MainMethod &method) {
  ir_type *mainMethodType =
      new_type_method(0, 0, false, cc_cdecl_set, mtp_no_property);

  ir_entity *entity =
      new_entity(get_glob_type(), new_id_from_str("main"), mainMethodType);

  ir_graph *mainMethodGraph = new_ir_graph(entity, 0);
  set_current_ir_graph(mainMethodGraph);

  method.acceptChildren(this);

  irg_finalize_cons(mainMethodGraph);
}

void FirmVisitor::visitRegularMethod(ast::RegularMethod &method) {
  int numReturnValues = method.getReturnType()->getSemaType().isVoid() ? 0 : 1;
  auto parameters = method.getParameters();
  ir_type *methodType = new_type_method(parameters.size() + 1, numReturnValues,
                                        false, cc_cdecl_set, mtp_no_property);

  if (numReturnValues > 0)
    set_method_res_type(methodType, 0, intType);

  set_method_param_type(methodType, 0, this->currentClassType);

  int numParams = 1;
  for (auto &param : parameters) {
    set_method_param_type(methodType, numParams, getIrType(param->getType()));
    numParams++;
  }

  ir_entity *entity =
      new_entity(this->currentClassType,
                 new_id_from_str(method.getName().c_str()), methodType);

  /* "returns a new graph consisting of a start block, a regular block
   * and an end block" */
  ir_graph *methodGraph = new_ir_graph(entity,
                                       numParams); // number of local variables including parameters
  set_current_ir_graph(methodGraph);

  // Add projections for arguments
  ir_node *lastBlock = get_r_cur_block(methodGraph);
  // set the start block to be the current block
  set_r_cur_block(methodGraph, get_irg_start_block(methodGraph));
  ir_node *args = get_irg_args(methodGraph);

  int i = 0;
  for(auto &param : parameters) {
    (void)param;
    // TODO: Save this somewhere?
    new_Proj(args, mode_Is, i); // TODO: Correct mode
    i++;
  }

  set_r_cur_block(methodGraph, lastBlock);

  method.acceptChildren(this);

  // "... mature the current block, which means fixing the number of their predecessors"
  mature_immBlock(get_r_cur_block(methodGraph));

  irg_finalize_cons(methodGraph);
}

void FirmVisitor::visitClass(ast::Class &klass) {
  // Ignore
  (void)klass;
}

void FirmVisitor::visitReturnStatement(ast::ReturnStatement &stmt) {
  (void)stmt;
  ir_node *constRetval = new_Const_long(mode_Is, 0);
  ir_graph *currentGraph = get_current_ir_graph();

  ir_node *results[1] = {constRetval};

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
    // TODO: Implement
    assert(false);
  }
}

void FirmVisitor::visitIntLiteral(ast::IntLiteral &lit) {
  // We only have signed 32 bit integers.
  ir_node *node = new_Const(new_tarval_from_long(lit.getValue(), mode_Is));
  pushNode(node);
}
