#include "asm_pass.hpp"

#include <fstream>
#include <cstring>

#define ORDER


#ifdef ORDER
#define PRINT_ORDER ir_printf("%s: %n %N\n", __FUNCTION__, node, node);
#else
#define PRINT_ORDER ;
#endif

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

std::string getProjLabel(ir_node *node) {
  assert(is_Proj(node));
  assert(get_irn_mode(node) == mode_X);
  if (get_Proj_num(node) == pn_Cond_true)
    return "true_" + std::to_string(get_irn_node_nr(node));
  else if(get_Proj_num(node) == pn_Cond_false)
    return "false_" + std::to_string(get_irn_node_nr(node));

  assert(false);
}

std::string nodeStr(ir_node *node) {
  return std::string(gdb_node_helper(node)) + " " + std::to_string(get_irn_node_nr(node));
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
  PRINT_ORDER;
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
  PRINT_ORDER;
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
  PRINT_ORDER;
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

void AsmMethodPass::visitDiv(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  auto regMode = Asm::X86Reg::getRegMode(node);
  auto leftOp = getNodeResAsInstOperand(get_Div_left(node));
  auto rightOp = getNodeResAsInstOperand(get_Div_right(node));

  // Move left into rax
  auto axOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(leftOp), std::move(axOp));

  // Right into rcx
  auto cxOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(rightOp), std::move(cxOp));

  // "the instruction cqto is used to perform sign extension,
  //  copying the sign bit of %rax into every bit of %rdx."
  bb->emplaceInstruction<Asm::Cqto>();

  // Div only takes one argument, divides rax by that and stores the result in rax
  cxOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Div>(std::move(cxOp));


  ir_node *succ = getSucc(node, iro_Proj, mode_Ls);
  assert(is_Proj(succ));

  auto dstReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, regMode));
  writeValue(std::move(dstReg), succ);
}

void AsmMethodPass::visitMod(ir_node *node) {
  PRINT_ORDER;
  // Do the exact same thing we do in visitDiv, but use the value in dx in the end,
  // which contains the remainder.
  auto bb = getBB(node);

  auto regMode = Asm::X86Reg::getRegMode(node);
  auto leftOp = getNodeResAsInstOperand(get_Mod_left(node));
  auto rightOp = getNodeResAsInstOperand(get_Mod_right(node));

  // Move left into rax
  auto axOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(leftOp), std::move(axOp));

  // Right into rcx
  auto cxOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Mov>(std::move(rightOp), std::move(cxOp));

  // "the instruction cqto is used to perform sign extension,
  //  copying the sign bit of %rax into every bit of %rdx."
  bb->emplaceInstruction<Asm::Cqto>();

  // Div only takes one argument, divides rax by that and stores the result in rax
  cxOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, Asm::X86Reg::Mode::R));
  bb->emplaceInstruction<Asm::Div>(std::move(cxOp));


  ir_node *succ = getSucc(node, iro_Proj, mode_Ls);
  assert(is_Proj(succ));

  auto dstReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::dx, regMode));
  writeValue(std::move(dstReg), succ);
}

void AsmMethodPass::visitMul(ir_node *node) {
  PRINT_ORDER;
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
  PRINT_ORDER;
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
    //auto di = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::getRegMode(p)));
    auto di = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::Mode::R));

    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(p), std::move(di));

  } else if (strcmp(funcName, "allocate") == 0) {
    int nParams = get_Call_n_params(node);
    assert(nParams == 2);
    ir_node *n = get_Call_param(node, 0);
    ir_node *size = get_Call_param(node, 1);
    //ir_printf("Allocate params: %n %N, %n %N\n", n, n, size, size);
    auto di = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::di, Asm::X86Reg::getRegMode(n)));
    auto si = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::si, Asm::X86Reg::getRegMode(size)));

    // num in rdi
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(n), std::move(di));
    // size in rsi
    bb->emplaceInstruction<Asm::Mov>(getNodeResAsInstOperand(size), std::move(si));
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
        //auto reg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax,
                                      //Asm::X86Reg::getRegMode(resultProj)));
        writeValue(std::move(reg), resultProj, "Return value of " + std::string(funcName));
      }
    }
  }
}

void AsmMethodPass::visitCmp(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  auto leftRegMode = Asm::X86Reg::getRegMode(get_Cmp_left(node));
  auto rightRegMode = Asm::X86Reg::getRegMode(get_Cmp_right(node));

  auto leftReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, leftRegMode));
  auto rightReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::bx, rightRegMode));

  // Load left into ax, right into bx
  auto leftOp = getNodeResAsInstOperand(get_Cmp_left(node));
  bb->emplaceInstruction<Asm::Mov>(std::move(leftOp), std::move(leftReg), leftRegMode);

  auto rightOp = getNodeResAsInstOperand(get_Cmp_right(node));
  bb->emplaceInstruction<Asm::Mov>(std::move(rightOp), std::move(rightReg), rightRegMode);

  leftReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, leftRegMode));
  rightReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::bx, rightRegMode));
  /* left and right swapped! */
  bb->emplaceInstruction<Asm::Cmp>(std::move(rightReg), std::move(leftReg));
}

void AsmMethodPass::visitCond(ir_node *node) {
  PRINT_ORDER;
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


  if (falseBlock == trueBlock) {
    // Successor block is the same for both true and false case. This is a boolean comparison.
    // We Jump to the labels of the true/false proj nodes, which don't exist yet but will
    // later when the Phi node generates them.
    bb->emplaceInstruction<Asm::Jmp>(getProjLabel(trueProj),
                                     relation);

    bb->emplaceInstruction<Asm::Jmp>(getProjLabel(falseProj),
                                     getInverseRelation(relation));
  } else {
    // Control flow comparison, jump to the appropriate basic block
    bb->emplaceInstruction<Asm::Jmp>(getBlockLabel(trueBlock),
                                     relation);

    bb->emplaceInstruction<Asm::Jmp>(getBlockLabel(falseBlock),
                                     getInverseRelation(relation));
  }
}

void AsmMethodPass::visitJmp(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  ir_node *jumpTarget = getNthSucc(node, 0);
  assert(is_Block(jumpTarget));

  bb->emplaceJump(getBlockLabel(jumpTarget), ir_relation_true);
}

void AsmMethodPass::visitEnd(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  bb->emplaceJump(func->getEpilogLabel(), ir_relation_true);
}


void AsmMethodPass::visitReturn(ir_node *node) {
  auto bb = getBB(node);

  ir_node *succ = getNthSucc(node, 0);
  assert(is_Block(succ));

  if (get_Return_n_ress(node) > 0) {
    ir_node *opNode = get_Return_res(node, 0);
    auto op = getNodeResAsInstOperand(opNode);
    auto regMode = Asm::X86Reg::getRegMode(opNode);
    auto ax = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::ax, regMode));

    bb->emplaceInstruction<Asm::Mov>(std::move(op), std::move(ax), regMode, "Return value");
  }

  // Jump to end block
  // return nodes should have exactly one successor, the end block.
  bb->emplaceJump(getBlockLabel(succ), ir_relation_true);
}

void AsmMethodPass::visitLoad(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  ir_node *pred = get_Load_ptr(node);
  ir_node *succ;

  // Load nodes have 2 successors, one Proj M and another one, which we want.
  succ = getNthSucc(node, 0);
  if (get_irn_mode(succ) == mode_M)
    succ =getNthSucc(node, 1);

  if (succ == nullptr) {
    std::cout << nodeStr(node) << " has no successor" << std::endl;
    // We load something but don't end up using it at all?
    return;
  }

  assert(get_irn_mode(succ) != mode_M); // ! Load nodes have 2 successor Proj nodes
  // TODO: Do we need this on non-pointer nodes?
  assert(get_irn_mode(pred) == mode_P);

  bb->addComment("Load from " + nodeStr(pred));
  /*
   * 1) Load contents of pred slot into temporary register. This now contains the address to read from
   * 2) Write the value at that address into a second temporary register
   * 3) Write that value to succ's slot
   */
  auto predRegMode = Asm::X86Reg::getRegMode(pred);
  auto succRegMode = Asm::X86Reg::getRegMode(succ);
  std::cout << "Pred Reg Mode: " << predRegMode << std::endl;
  std::cout << "Succ Reg Mode  : " << succRegMode << std::endl;

  // 1)
  auto predOp = getNodeResAsInstOperand(pred);
  auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, predRegMode));
  bb->emplaceInstruction<Asm::Mov>(std::move(predOp), std::move(tmpReg), predRegMode, "1)");

  // 2)
  auto cxOp = std::make_unique<Asm::MemoryBase>(0, Asm::X86Reg(Asm::X86Reg::Name::cx, predRegMode));
  auto dxOp = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::dx, succRegMode));
  bb->emplaceInstruction<Asm::Mov>(std::move(cxOp), std::move(dxOp), succRegMode, "2)");

  // 3)
  auto val = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::dx, Asm::X86Reg::Mode::R));
  writeValue(std::move(val), succ, "3)");
}


void AsmMethodPass::visitStore(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  ir_node *source = get_Store_value(node);
  ir_node *dest = get_Store_ptr(node);

  bb->addComment("Store(" + nodeStr(node) + "): " + nodeStr(source) + " into " + nodeStr(dest));

  // dest is a pointer, so we need to save into the address of that pointer,
  // not into the register of that node.
  assert(get_irn_mode(dest) == mode_P);

  /*
   * 1) Copy contents from DEST register into tmp register (will contain address)
   * 2) write SOURCE value into address in tmp register.
   */

  auto sourceRegMode = Asm::X86Reg::getRegMode(source);
  auto destRegMode = Asm::X86Reg::getRegMode(dest);
  std::cout << "Source Reg Mode: " << sourceRegMode << std::endl;
  std::cout << "Dest Reg Mode  : " << destRegMode << std::endl;

  auto destOp = getNodeResAsInstOperand(dest);
  auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::dx, destRegMode));
  bb->emplaceInstruction<Asm::Mov>(std::move(destOp), std::move(tmpReg), destRegMode, "1)");
  // r15 now contains the address to write to!

  // Load value to write to dest into tmp register
  auto r14Op = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, destRegMode));
  auto sourceOp = getNodeResAsInstOperand(source);
  bb->emplaceInstruction<Asm::Mov>(std::move(sourceOp), std::move(r14Op), destRegMode, "2)");

  // Mov %cx into (%dx)
   r14Op = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::cx, sourceRegMode));
  auto r15Op = std::make_unique<Asm::MemoryBase>(0, Asm::X86Reg(Asm::X86Reg::Name::dx,
                                                 Asm::X86Reg::Mode::R)); // Pointer, force R
  bb->emplaceInstruction<Asm::Mov>(std::move(r14Op), std::move(r15Op), sourceRegMode, "3)");

}

void AsmMethodPass::visitPhi(ir_node *node) {
  PRINT_ORDER;
  if (get_Phi_loop(node))
    return; // ???

  int nPreds = get_Phi_n_preds(node);
  assert(nPreds == 2);
  std::string phiLabel = "phi_" + std::to_string(get_irn_node_nr(node));

  bool needsLabels = get_nodes_block(get_Block_cfgpred(get_nodes_block(node), 0)) ==
                     get_nodes_block(get_Block_cfgpred(get_nodes_block(node), 1));

  auto bb = getBB(node);
  bb->addComment("Phi: " + std::to_string(needsLabels));
  for (int i = 0; i < nPreds; i ++) {
    ir_node *phiPred = get_Phi_pred(node, i);
    ir_node *blockPredNode = get_Block_cfgpred(get_nodes_block(node), i);
    ir_node *blockPred = get_nodes_block(blockPredNode);

    ir_printf("%d Phi pred  : %n %N\n", i, phiPred, phiPred);
    ir_printf("%d Block Pred NODE: %n %N\n", i, blockPredNode, blockPredNode);
    ir_printf("%d Block Pred: %n %N\n", i, blockPred, blockPred);


    if (needsLabels) {
      /* bool value phi. Create labels for true/false cases and a jump to the phi label below */
      assert(is_Proj(blockPredNode));
      assert(get_irn_mode(blockPredNode) == mode_X);

      bb->emplaceInstruction<Asm::Label>(std::move(getProjLabel(blockPredNode)));
      // Write this phiPred into the stack slot of the phi node
      auto srcOp = getNodeResAsInstOperand(phiPred);

      auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
      bb->emplaceInstruction<Asm::Mov>(std::move(srcOp), std::move(tmpReg),
                                    Asm::X86Reg::Mode::R, "phi tmp");

      // This is basically writeValue but the basic block is not the one of the passed node!
      auto dstOp = std::make_unique<Asm::MemoryBase>(ssm.getStackSlot(node, bb),
                                                     Asm::X86Reg(Asm::X86Reg::Name::bp,
                                                                 Asm::X86Reg::Mode::R));
      tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
      bb->emplaceInstruction<Asm::Mov>(std::move(tmpReg), std::move(dstOp),
                                    Asm::X86Reg::Mode::R, "phi dst");

      bb->emplaceInstruction<Asm::Jmp>(phiLabel, ir_relation_true);
    } else {
      bb = getBB(blockPred);
      /* Control flow, generate phi instructions in the predecessor basic blocks */
      // Write this phiPred into the stack slot of the phi node
      auto srcOp = getNodeResAsInstOperand(phiPred);

      auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
      bb->emplacePhiInstr<Asm::Mov>(std::move(srcOp), std::move(tmpReg),
                                    Asm::X86Reg::Mode::R, "phi tmp");

      // This is basically writeValue but the basic block is not the one of the passed node!
      auto dstOp = std::make_unique<Asm::MemoryBase>(ssm.getStackSlot(node, bb),
                                                     Asm::X86Reg(Asm::X86Reg::Name::bp,
                                                                 Asm::X86Reg::Mode::R));
      tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, Asm::X86Reg::Mode::R));
      bb->emplacePhiInstr<Asm::Mov>(std::move(tmpReg), std::move(dstOp),
                                    Asm::X86Reg::Mode::R, "phi dst");
    }
  }

  if (needsLabels)
    bb->emplaceInstruction<Asm::Label>(phiLabel);
}

void AsmMethodPass::visitMinus(ir_node *node) {
  PRINT_ORDER;
  ir_node *sourceNode = get_Minus_op(node);
  auto bb = getBB(node);
  auto regMode = Asm::X86Reg::getRegMode(node);
  bb->addComment("Negate " + nodeStr(sourceNode) + " into " + nodeStr(node));
  auto sourceOp = getNodeResAsInstOperand(sourceNode);

  auto tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, regMode));
  bb->emplaceInstruction<Asm::Mov>(std::move(sourceOp), std::move(tmpReg));

  tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, regMode));
  bb->emplaceInstruction<Asm::Neg>(std::move(tmpReg), "Negate " + nodeStr(sourceNode));

  tmpReg = Asm::Register::get(Asm::X86Reg(Asm::X86Reg::Name::r15, regMode));
  auto destOp = getNodeResAsInstOperand(node);
  bb->emplaceInstruction<Asm::Mov>(std::move(tmpReg), std::move(destOp), regMode);
}
