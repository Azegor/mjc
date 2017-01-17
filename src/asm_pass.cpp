#include "asm_pass.hpp"

#include <fstream>
#include <cstring>

void AsmPass::before() {
  // writer.writeTextSection();
}

void AsmPass::visitMethod(ir_graph *graph) {
  const char *functionName = get_entity_ld_name(get_irg_entity(graph));
  Asm::Function func(functionName);
  AsmMethodPass methodPass(graph, &func);
  methodPass.run();
  asmProgram.addFunction(std::move(func));
}

void AsmPass::after() {
  std::ofstream outputFile(outputFileName);
  if (!outputFile.is_open()) {
    throw std::runtime_error("could not open file '" + outputFileName + '\'');
  }
  outputFile << asmProgram << std::endl;
}

void AsmMethodPass::visitAdd(ir_node *node) {
  auto bb = getBB(node);
  // 1. get left and right predecessor of add node
  // 2. get values from predecessor. Either immediate or generated val (stack slot)
  // 3. create memory or immediate operands
  // 4. create instruction with operands

  // TODO cleanup and avoid code duplication in other visit Methods
  auto regMode = Asm::X86Reg::getRegMode(get_irn_mode(node));
  auto leftOp = getNodeResAsInstOperand(get_Add_left(node));
  Asm::X86Reg leftReg(Asm::X86Reg::Name::ax, regMode);
  auto leftRegInst = loadToReg(std::move(leftOp), leftReg);
  bb->addInstruction(std::move(leftRegInst));
  auto rightOp = getNodeResAsInstOperand(get_Add_right(node));
  Asm::X86Reg rightReg(Asm::X86Reg::Name::bx, regMode);
  auto rightRegInst = loadToReg(std::move(rightOp), rightReg);
  bb->addInstruction(std::move(rightRegInst));
  //   if (auto rWriteOp = dynamic_cast<Asm::WritableOperand *>(rightOp.get())) {
  bb->emplaceInstruction<Asm::Add>(Asm::Register::get(leftReg), Asm::Register::get(rightReg),
                                   "Node " + std::to_string(get_irn_node_nr(node)));
  bb->addInstruction(writeResToStackSlot(rightReg, node));
}

void AsmMethodPass::visitCall(ir_node *node) {
  auto bb = getBB(node);

  assert(bb != nullptr);

  ir_node *address = get_Call_ptr(node);
  ir_type *callType = get_Call_type(node);
  assert(is_Address(address));
  ir_entity *entity = get_Address_entity(address);
  const char *funcName = get_entity_name(entity);
  std::cout << funcName << std::endl;

  // address nodes are always constant and belong to the start block
  //TODO Arguments

  if (strcmp(funcName, "print_int") == 0 ||
      strcmp(funcName, "write_int") == 0) {
    int nParams = get_Call_n_params(node);
    assert(nParams == 1);
    ir_node *p = get_Call_param(node, 0);
    ir_printf("call param: %n %N\n", p, p);
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(p),
                                     Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::Mode::R)));

  }

  bb->emplaceInstruction<Asm::Call>(std::string(funcName));

  // Write result of function call into stack slot of call node
  if (get_method_n_ress(callType) > 0) {
    assert(get_method_n_ress(callType) == 1);
    ir_node *resultProj = nullptr;

    foreach_out_edge_safe(node, edge) {
      //ir_printf("%n -> %n\n", node, get_edge_src_irn(edge));
      ir_node *src = get_edge_src_irn(edge);
      if (is_Proj(src) && get_irn_mode(src) == mode_T) {
        // result projection, should have exactly one successor, another Proj node.
        foreach_out_edge_safe(src, _edge) {
          ir_node *_src = get_edge_src_irn(_edge);
          assert(is_Proj(_src));
          resultProj = _src;
          break;
        }
      }
    }

    if (resultProj != nullptr) {
      auto reg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, Asm::X86Reg::Mode::R));
      writeValue(std::move(reg), resultProj);
    }
  }
}
