#include "firm_visitor.hpp"

FirmVisitor::FirmVisitor() {
  ir_init();

  intType = new_type_primitive(mode_Is);
  boolType = new_type_primitive(mode_Bu);
  arrayType = new_type_primitive(mode_P);
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
    (void)param;
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

  method.acceptChildren(this);

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
