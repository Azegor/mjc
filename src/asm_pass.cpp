#include "asm_pass.hpp"

#include <fstream>
#include <cstring>

/* Return first Proj suceessor of @node */
ir_node * getProjSucc(ir_node *node) {
  foreach_out_edge_safe(node, edge) {
    ir_node *src = get_edge_src_irn(edge);
    if (is_Proj(src))
      return src;
  }
  return nullptr;
}

/* Return first successor node with the given opcode and the given mode */
ir_node *getSucc(ir_node *node, unsigned opcode, ir_mode *mode) {
  foreach_out_edge_safe(node, edge) {
    ir_node *src = get_edge_src_irn(edge);

    if (get_irn_opcode(src) == opcode &&
        get_irn_mode(src) == mode)
      return src;
  }

  return nullptr;
}

ir_node *getNthSucc(ir_node *node, int k) {
  int i = 0;
  foreach_out_edge_safe(node, edge) {
    if (i < k) {
      i ++;
      continue;
    }
    ir_node *src = get_edge_src_irn(edge);
    return src;
  }

  return nullptr;
}

ir_relation getInverseRelation(ir_relation relation) {
  switch(relation) {
    case ir_relation_equal:
      return ir_relation_less_greater;
    case ir_relation_less_greater:
      return ir_relation_equal;
    case ir_relation_greater:
      return ir_relation_less_equal;
    case ir_relation_less:
      return ir_relation_greater_equal;
    case ir_relation_less_equal:
      return ir_relation_greater;
    case ir_relation_greater_equal:
      return ir_relation_less;
    default:
      assert(0);
  }
}


void AsmPass::before() {
  // writer.writeTextSection();
}

void AsmPass::visitMethod(ir_graph *graph) {
  const char *functionName = get_entity_ld_name(get_irg_entity(graph));
  Asm::Function func(functionName);
  func.setStartBlockId(get_irn_node_nr(get_irg_start_block(graph)));
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

  } else if (strcmp(funcName, "allocate") == 0) {
    int nParams = get_Call_n_params(node);
    assert(nParams == 2);
    ir_node *n = get_Call_param(node, 0);
    ir_node *size = get_Call_param(node, 1);
    ir_printf("Allocate params: %n %N, %n %N\n", n, n, size, size);

    // num in rdi
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(n),
                                     Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::Mode::R)));
    // size in rsi
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(size),
                                     Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::si, Asm::X86Reg::Mode::R)));


  }

  bb->emplaceInstruction<Asm::Call>(std::string(funcName));

  // Write result of function call into stack slot of call node
  if (get_method_n_ress(callType) > 0) {
    assert(get_method_n_ress(callType) == 1);

    ir_node *projSucc = getSucc(node, iro_Proj, mode_T);
    ir_node *resultProj = getProjSucc(projSucc);

    if (resultProj != nullptr) {
      auto reg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, Asm::X86Reg::Mode::R));
      writeValue(std::move(reg), resultProj);
    }
  }
}

void AsmMethodPass::visitCmp(ir_node *node) {
  auto bb = getBB(node);
  auto regMode = Asm::X86Reg::getRegMode(get_irn_mode(node));
  auto leftOp = getNodeResAsInstOperand(get_Cmp_left(node));
  Asm::X86Reg leftReg(Asm::X86Reg::Name::ax, regMode);
  auto leftRegInst = loadToReg(std::move(leftOp), leftReg);
  bb->addInstruction(std::move(leftRegInst));

  auto rightOp = getNodeResAsInstOperand(get_Cmp_right(node));
  Asm::X86Reg rightReg(Asm::X86Reg::Name::bx, regMode);
  auto rightRegInst = loadToReg(std::move(rightOp), rightReg);
  bb->addInstruction(std::move(rightRegInst));

  /* left and right swapped! */
  bb->emplaceInstruction<Asm::Cmp>(Asm::Register::get(rightReg),
                                   Asm::Register::get(leftReg));
}

void AsmMethodPass::visitCond(ir_node *node) {
  auto bb = getBB(node);

  ir_node *selector = get_Cond_selector(node);
  ir_relation relation = get_Cmp_relation(selector);
  assert(is_Cmp(selector));

  ir_node *falseProj = getNthSucc(node, 0);
  ir_node *trueProj  = getNthSucc(node, 1);

  if (get_Proj_num(falseProj) != pn_Cond_false) {
    assert(get_Proj_num(trueProj) == pn_Cond_false);
    std::swap(falseProj, trueProj);
  }

  assert(is_Proj(falseProj));
  assert(get_Proj_num(falseProj) == pn_Cond_false);
  assert(get_Proj_num(trueProj) == pn_Cond_true);

  ir_node *falseBlock = getNthSucc(falseProj, 0);
  assert(is_Block(falseBlock));
  ir_node *trueBlock = getNthSucc(trueProj, 0);
  assert(is_Block(trueBlock));

  bb->emplaceInstruction<Asm::Jmp>("L" + std::to_string(get_irn_node_nr(trueBlock)),
                                   relation);


  bb->emplaceInstruction<Asm::Jmp>("L" + std::to_string(get_irn_node_nr(falseBlock)),
                                   getInverseRelation(relation));
}

void AsmMethodPass::visitJmp(ir_node *node) {
  auto bb = getBB(node);
  ir_node *jumpTarget = getNthSucc(node, 0);
  assert(is_Block(jumpTarget));
  std::string targetLabel = "L" + std::to_string(get_irn_node_nr(jumpTarget));

  bb->emplaceJump<Asm::Jmp>(targetLabel, ir_relation_true);
}
