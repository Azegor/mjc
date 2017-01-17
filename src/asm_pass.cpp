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

std::string getBlockLabel(ir_node *node) {
  assert(is_Block(node));

  return "L" + std::to_string(get_irn_node_nr(node));
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

void AsmMethodPass::visitConv(ir_node *node) {
  //auto bb = getBB(node);
  ir_node *pred = get_Conv_op(node);
  if (is_Const(pred)) {
    // We already handle this simple case in getNodeResAsInstOperand
    return;
  }

  // Just use pred's stack slot for this Conv node as well.
  ssm.copySlot(pred, node);
  // TODO: Depending on the conversion, this might actually promote a
  //       value from e.g. L to R mode.
}

void AsmMethodPass::visitAdd(ir_node *node) {
  auto bb = getBB(node);
  // 1. get left and right predecessor of add node
  // 2. get values from predecessor. Either immediate or generated val (stack slot)
  // 3. create memory or immediate operands
  // 4. create instruction with operands

  // TODO cleanup and avoid code duplication in other visit Methods
  auto regMode = Asm::X86Reg::getRegMode(node);
  auto leftOp = getNodeResAsInstOperand(get_Add_left(node));
  Asm::X86Reg leftReg(Asm::X86Reg::Name::ax, regMode);
  auto leftRegInst = loadToReg(std::move(leftOp), leftReg);
  bb->addInstruction(std::move(leftRegInst));
  auto rightOp = getNodeResAsInstOperand(get_Add_right(node));
  Asm::X86Reg rightReg(Asm::X86Reg::Name::bx, regMode);
  auto rightRegInst = loadToReg(std::move(rightOp), rightReg);
  bb->addInstruction(std::move(rightRegInst));
  bb->emplaceInstruction<Asm::Add>(Asm::Register::get(leftReg), Asm::Register::get(rightReg),
                                   "Node " + std::to_string(get_irn_node_nr(node)));
  bb->addInstruction(writeResToStackSlot(rightReg, node));
}

void AsmMethodPass::visitSub(ir_node *node) {
  auto bb = getBB(node);

  auto regMode = Asm::X86Reg::getRegMode(node);
  auto leftOp = getNodeResAsInstOperand(get_Sub_left(node));
  Asm::X86Reg leftReg(Asm::X86Reg::Name::ax, regMode);
  auto leftRegInst = loadToReg(std::move(leftOp), leftReg);
  bb->addInstruction(std::move(leftRegInst));
  auto rightOp = getNodeResAsInstOperand(get_Sub_right(node));
  Asm::X86Reg rightReg(Asm::X86Reg::Name::bx, regMode);
  auto rightRegInst = loadToReg(std::move(rightOp), rightReg);
  bb->addInstruction(std::move(rightRegInst));

  // Left and right swapped!
  bb->emplaceInstruction<Asm::Sub>(Asm::Register::get(rightReg), Asm::Register::get(leftReg),
                                   "Node " + std::to_string(get_irn_node_nr(node)));
  bb->addInstruction(writeResToStackSlot(leftReg, node));
}

void AsmMethodPass::visitMul(ir_node *node) {
  auto bb = getBB(node);

  // TODO cleanup and avoid code duplication in other visit Methods
  auto regMode = Asm::X86Reg::getRegMode(node);
  auto leftOp = getNodeResAsInstOperand(get_Mul_left(node));
  Asm::X86Reg leftReg(Asm::X86Reg::Name::ax, regMode);
  auto leftRegInst = loadToReg(std::move(leftOp), leftReg);
  bb->addInstruction(std::move(leftRegInst));
  auto rightOp = getNodeResAsInstOperand(get_Mul_right(node));
  Asm::X86Reg rightReg(Asm::X86Reg::Name::bx, regMode);
  auto rightRegInst = loadToReg(std::move(rightOp), rightReg);
  bb->addInstruction(std::move(rightRegInst));
  bb->emplaceInstruction<Asm::Mul>(Asm::Register::get(leftReg), Asm::Register::get(rightReg),
                                   "Node " + std::to_string(get_irn_node_nr(node)));
  bb->addInstruction(writeResToStackSlot(rightReg, node));
}

void AsmMethodPass::visitCall(ir_node *node) {
  auto bb = getBB(node);

  ir_node *address = get_Call_ptr(node);
  ir_type *callType = get_Call_type(node);
  assert(is_Address(address));
  ir_entity *entity = get_Address_entity(address);
  const char *funcName = get_entity_name(entity);

  // address nodes are always constant and belong to the start block
  //TODO Arguments

  if (strcmp(funcName, "print_int") == 0 ||
      strcmp(funcName, "write_int") == 0) {
    int nParams = get_Call_n_params(node);
    assert(nParams == 1);
    ir_node *p = get_Call_param(node, 0);
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(p),
                                     Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::Mode::R)));

  } else if (strcmp(funcName, "allocate") == 0) {
    int nParams = get_Call_n_params(node);
    assert(nParams == 2);
    ir_node *n = get_Call_param(node, 0);
    ir_node *size = get_Call_param(node, 1);
    //ir_printf("Allocate params: %n %N, %n %N\n", n, n, size, size);

    // num in rdi
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(n),
                                     Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::Mode::R)));
    // size in rsi
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(size),
                                     Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::si, Asm::X86Reg::Mode::R)));


  }

  bb->emplaceInstruction<Asm::Call>(std::string(funcName));

  // Write result of function call into stack slot of call->proj->proj node
  if (get_method_n_ress(callType) > 0) {
    assert(get_method_n_ress(callType) == 1);

    ir_node *projSucc = getSucc(node, iro_Proj, mode_T);
      if (projSucc != nullptr) {
      ir_node *resultProj = getProjSucc(projSucc);

      if (resultProj != nullptr) {
        auto reg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, Asm::X86Reg::Mode::R));
        writeValue(std::move(reg), resultProj);
      }
    }
  }
}

void AsmMethodPass::visitCmp(ir_node *node) {
  auto bb = getBB(node);
  auto regMode = Asm::X86Reg::getRegMode(get_Cmp_right(node));
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

  bb->emplaceInstruction<Asm::Jmp>(getBlockLabel(trueBlock),
                                   relation);


  bb->emplaceInstruction<Asm::Jmp>(getBlockLabel(falseBlock),
                                   getInverseRelation(relation));
}

void AsmMethodPass::visitJmp(ir_node *node) {
  auto bb = getBB(node);
  ir_node *jumpTarget = getNthSucc(node, 0);
  assert(is_Block(jumpTarget));

  bb->emplaceJump(getBlockLabel(jumpTarget), ir_relation_true);
}

void AsmMethodPass::visitEnd(ir_node *node) {
  auto bb = getBB(node);

  bb->emplaceJump(func->getEpilogLabel(), ir_relation_true);
}

void AsmMethodPass::visitLoad(ir_node *node) {
  auto bb = getBB(node);
  ir_node *pred = get_Load_ptr(node);
  ir_node *succ = getSucc(node, iro_Proj, mode_Is);

  if (succ == nullptr) {
    // We load something but don't end up using it at all?
    return;
  }

  assert(get_irn_mode(node) != mode_M); // ! Load nodes have 2 successor Proj nodes
  // TODO: Do we need this on non-pointer nodes?
  assert(get_irn_mode(pred) == mode_P);

  bb->addComment("Load from " + std::string(gdb_node_helper(pred)));
  /*
   * 1) Load contents of pred slot into temporary register. This now contains the address to read from
   * 2) Write the value at that address into a second temporary register
   * 3) Write that value to succ's slot
   */

  // 1)
  auto predOp = getNodeResAsInstOperand(pred);
  auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(predOp), std::move(tmpReg), "1)");

  // 2)
  auto r15Op = std::make_unique<Asm::MemoryBase>(0, Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
  auto r14Op = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r14, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(r15Op), std::move(r14Op), "2)");

  // 3)
  auto val =  Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r14, Asm::X86Reg::Mode::R));
  writeValue(std::move(val), succ, "3)");
}

void AsmMethodPass::visitReturn(ir_node *node) {
  auto bb = getBB(node);

  ir_node *succ = getNthSucc(node, 0);
  assert(is_Block(succ));

  if (get_Return_n_ress(node) > 0) {
    ir_node *opNode = get_Return_res(node, 0);
    auto op = getNodeResAsInstOperand(opNode);
    auto ax = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax,
                                 Asm::X86Reg::getRegMode(opNode)));

    bb->emplaceInstruction<Asm::Mov>(std::move(op), std::move(ax), "Return value");
  }

  // Jump to end block
  // return nodes should have exactly one successor, the end block.
  bb->emplaceJump(getBlockLabel(succ), ir_relation_true);
}

void AsmMethodPass::visitStore(ir_node *node) {
  auto bb = getBB(node);
  ir_node *source = get_Store_value(node);
  ir_node *dest = get_Store_ptr(node);

  bb->addComment("Store " + std::string(gdb_node_helper(source)) + " into " +
                 std::string(gdb_node_helper(dest)));

  // dest is a pointer, so we need to save into the address of that pointer,
  // not into the register of that node.
  assert(get_irn_mode(dest) == mode_P);

  /*
   * 1) Copy contents from DEST register into tmp register (will contain address)
   * 2) write SOURCE value into address in tmp register.
   */

  // TODO: Use the correct register modes here
  auto destOp = getNodeResAsInstOperand(dest);
  auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(destOp), std::move(tmpReg), "1)");
  // r15 now contains the address to write to!

  auto r15Op = std::make_unique<Asm::MemoryBase>(0, Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::L));
  auto sourceOp = getNodeResAsInstOperand(source);
  bb->emplaceInstruction<Asm::Mov>(std::move(sourceOp), std::move(r15Op), "2)");
}

void AsmMethodPass::visitPhi(ir_node *node) {
  //ir_printf("%s: %n %N\n", __FUNCTION__, node, node);
  if (get_Phi_loop(node))
    return; // ???

  int nPreds = get_Phi_n_preds(node);

  for (int i = 0; i < nPreds; i ++) {
    ir_node *phiPred = get_Phi_pred(node, i);
    ir_node *blockPredNode = get_Block_cfgpred(get_nodes_block(node), i);
    ir_node *blockPred = get_nodes_block(blockPredNode);

    auto bb = getBB(blockPred);
    // Write this phiPred into the stack slot of the phi node
    auto srcOp = getNodeResAsInstOperand(phiPred);

    auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
    bb->emplaceInstruction<Asm::Mov>(std::move(srcOp), std::move(tmpReg));

    /*
     * TODO: All the special-casing for tmp registers seems stupid, we should
     *       instead have a helper that generates a tmp mov instruction in case
     *       both source and dest are stack slots
     */

    // This is basically writeValue but the basic block is not the one of the passed node!
    auto dstOp = std::make_unique<Asm::MemoryBase>(ssm.getStackSlot(node, bb),
                                                   Asm::X86Reg(Asm::X86Reg::Name::bp,
                                                               Asm::X86Reg::getRegMode(node)));
    tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
    bb->emplaceInstruction<Asm::Mov>(std::move(tmpReg), std::move(dstOp), "phi dst");
  }
}
