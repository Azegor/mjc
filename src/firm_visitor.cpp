#include "firm_visitor.hpp"

FirmVisitor::FirmVisitor() {
  ir_init();

  intType  = new_type_primitive (mode_Is);
  voidType = new_type_primitive (mode_Is);
  boolType = new_type_primitive (mode_Bu);
}

void FirmVisitor::visitProgram(ast::Program &program) {
  auto &classes = program.getClasses();

  // First, collect all class types
  for(auto &klass : classes) {
    ir_type *classType = new_type_class(klass->getName().c_str());
    this->classTypes[klass.get()] = classType;
  }

  for(auto &klass : classes) {
    this->currentClassType = classTypes[klass.get()];
    klass->acceptChildren(this);
  }

  dump_all_ir_graphs("");
}

void FirmVisitor::visitMainMethod(ast::MainMethod &method) {
  ir_type *mainMethodType = new_type_method(0, 0, false, cc_cdecl_set,
                                            mtp_no_property);

  ir_entity *entity = new_entity(get_glob_type(),
                                 new_id_from_str("main"),
                                 mainMethodType);

  ir_graph *mainMethodGraph = new_ir_graph(entity, 0);
  set_current_ir_graph(mainMethodGraph);

  method.acceptChildren(this);

  irg_finalize_cons(mainMethodGraph);
}

void FirmVisitor::visitRegularMethod(ast::RegularMethod &method) {
  int numReturnValues = method.getReturnType()->getSemaType().isVoid() ? 0 : 1;
  auto parameters = method.getParameters();
  ir_type *methodType = new_type_method(parameters.size() + 1,
                                        numReturnValues,
                                        false, cc_cdecl_set,
                                        mtp_no_property);

  if (numReturnValues > 0)
    set_method_res_type(methodType, 0, intType);

  set_method_param_type(methodType, 0, this->currentClassType);

  int i = 0;
  for (auto &param : parameters) {
    (void)param;
    set_method_param_type(methodType, 1 + i,
                          getIrType(param->getType()));
    i ++;
  }

  ir_entity *entity = new_entity(this->currentClassType,
                                 new_id_from_str(method.getName().c_str()),
                                 methodType);

  ir_graph *mainMethodGraph = new_ir_graph(entity, 0);
  set_current_ir_graph(mainMethodGraph);

  method.acceptChildren(this);

  irg_finalize_cons(mainMethodGraph);
}


void FirmVisitor::visitClass(ast::Class &klass) {
  // Ignore
  (void)klass;
}
