#include "asm_pass.hpp"

#include <fstream>
#include <cstring>


/* Return first Proj suceessor of @node */
static ir_node * getProjSucc(ir_node *node) {
  foreach_out_edge_safe(node, edge) {
    ir_node *src = get_edge_src_irn(edge);
    if (is_Proj(src))
      return src;
  }
  return nullptr;
}

/* Return first successor node with the given opcode and the given mode */
static ir_node *getSucc(ir_node *node, unsigned opcode, ir_mode *mode) {
  foreach_out_edge_safe(node, edge) {
    ir_node *src = get_edge_src_irn(edge);

    if (get_irn_opcode(src) == opcode &&
        get_irn_mode(src) == mode)
      return src;
  }

  return nullptr;
}

static ir_node *getNthSucc(ir_node *node, int k) {
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

int getNumSuccessors(ir_node *node) {
  int i = 0;
  foreach_out_edge_safe(node, edge) {
    i ++;
  }

  return i;
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
  AsmMethodPass methodPass(graph, &func, this->optimize);
  methodPass.run();
  asmProgram.addFunction(std::move(func));
}

void AsmMethodPass::before() {
  // Callee side of a function call
  //std::cout << "### visiting function " << get_entity_ld_name(get_irg_entity(graph)) << std::endl;
  ir_type *method_type = get_entity_type(get_irg_entity(graph));
  assert(is_Method_type(method_type));
  ir_node *argProj = get_irg_args(graph);
  assert(is_Proj(argProj));

  int nSuccessors = getNumSuccessors(argProj);
  //std::cout << "Successors: " << nSuccessors << std::endl;
  assert(is_Anchor(getNthSucc(argProj, 0))); // We skip the first successor
  //int paramStackSize = get_method_n_params(method_type) * 8;
  //std::cout << "paramStackSize: " << paramStackSize << std::endl;

  for (int i = 1; i < nSuccessors; i ++) {
    ir_node *succ = getNthSucc(argProj, i); //O(n²)!
    // get_Proj_num gives us the argument index, regardless of whether they are actually being used
    //int slot = 8 + paramStackSize - (get_Proj_num(succ) * 8);
    int slot = 16 + (get_Proj_num(succ) * 8);
    //ir_printf ("Succ %d: %n %N, num: %d, slot: %d\n", i, succ, succ, get_Proj_num(succ), slot);
    ssm.setSlot(succ, slot);
  }
}

void AsmMethodPass::visitConv(ir_node *node) {
  PRINT_ORDER;
  ir_node *pred = get_Conv_op(node);
  if (is_Const(pred)) {
    // We already handle this simple case in getNodeOp
    return;
  }

  if (is_Unknown(pred)) {
    // Uninitialized memory
    return;
  }

  // Just use pred's stack slot for this Conv node as well.
  ssm.copySlot(pred, node);
}

void AsmMethodPass::visitAdd(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  ir_node *leftNode  = get_Add_left(node);
  ir_node *rightNode = get_Add_right(node);
  // We want the constant to always be left
  if (is_Const(rightNode)) {
    std::swap(leftNode, rightNode);
  }

  // Generate the right node first so the (maybe) constant left one is directly
  // before the Add instrudction (easier for optimizations)

  bb->pushInstr(Asm::Mov, getNodeOp(rightNode), Asm::rbx());
  bb->pushInstr(Asm::Add, getNodeOp(leftNode),  Asm::rbx());
  bb->pushInstr(Asm::Mov, Asm::rbx(), getNodeOp(node), nodeStr(node));
}

void AsmMethodPass::visitSub(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  // TODO: Register modes!
  bb->pushInstr(Asm::Mov, getNodeOp(get_Sub_left(node)), Asm::rbx());
  bb->pushInstr(Asm::Sub, getNodeOp(get_Sub_right(node)), Asm::rbx());
  bb->pushInstr(Asm::Mov, Asm::rbx(), getNodeOp(node));
}

void AsmMethodPass::visitDiv(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  auto regMode = Asm::getRegMode(node);
  auto leftOp = getNodeOp(get_Div_left(node));
  auto rightOp = getNodeOp(get_Div_right(node));

  // Move left into rax
  if (leftOp.type == Asm::OP_IMM) {
    bb->pushInstr(Asm::Mov, leftOp, Asm::rax());

    bb->pushInstr(Asm::Movslq,
                  Asm::Op(Asm::RegName::ax, Asm::RegMode::E),
                  Asm::rax());

  } else {
    bb->pushInstr(Asm::Movslq, leftOp, Asm::rax());
  }

  // Right into rcx
  if (rightOp.type == Asm::OP_IMM) {
    bb->pushInstr(Asm::Mov, rightOp, Asm::rcx());

    bb->pushInstr(Asm::Movslq,
                  Asm::Op(Asm::RegName::cx, Asm::RegMode::E),
                  Asm::rcx());

  } else {
    bb->pushInstr(Asm::Movslq, rightOp, Asm::rcx());
  }

  // "the instruction cqto is used to perform sign extension,
  //  copying the sign bit of %rax into every bit of %rdx."
  bb->pushInstr(Asm::Cqto);

  // Div only takes one argument, divides rax by that and stores the result in rax
  bb->pushInstr(Asm::Div, Asm::rcx());

  // division result is in rax
  ir_node *succ = getSucc(node, iro_Proj, mode_Ls);
  assert(is_Proj(succ));
  bb->pushInstr(Asm::Movq,
                Asm::Op(Asm::RegName::ax, regMode),
                getNodeOp(succ));
}

void AsmMethodPass::visitMod(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  auto regMode = Asm::getRegMode(node);
  auto leftOp = getNodeOp(get_Mod_left(node));
  auto rightOp = getNodeOp(get_Mod_right(node));

  // Move left into rax
  if (leftOp.type == Asm::OP_IMM) {
    bb->pushInstr(Asm::Mov, leftOp, Asm::rax());

    bb->pushInstr(Asm::Movslq,
                  Asm::Op(Asm::RegName::ax, Asm::RegMode::E),
                  Asm::rax());

  } else {
    bb->pushInstr(Asm::Movslq, leftOp, Asm::rax());
  }

  // Right into rcx
  if (rightOp.type == Asm::OP_IMM) {
    bb->pushInstr(Asm::Mov, rightOp, Asm::rcx());

    bb->pushInstr(Asm::Movslq,
                  Asm::Op(Asm::RegName::cx, Asm::RegMode::E),
                  Asm::rcx());

  } else {
    bb->pushInstr(Asm::Movslq, rightOp, Asm::rcx());
  }

  // "the instruction cqto is used to perform sign extension,
  //  copying the sign bit of %rax into every bit of %rdx."
  bb->pushInstr(Asm::Cqto);

  // Div only takes one argument, divides rax by that and stores the result in rax
  bb->pushInstr(Asm::Div, Asm::rcx());

  // division result is in rax
  ir_node *succ = getSucc(node, iro_Proj, mode_Ls);
  assert(is_Proj(succ));
  bb->pushInstr(Asm::Movq,
                Asm::Op(Asm::RegName::dx, regMode),
                getNodeOp(succ));
}

void AsmMethodPass::visitMul(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  bb->pushInstr(Asm::Mov, getNodeOp(get_Mul_right(node)), Asm::rbx());
  bb->pushInstr(Asm::IMul, getNodeOp(get_Mul_left(node)), Asm::rbx());
  bb->pushInstr(Asm::Mov, Asm::rbx(), getNodeOp(node));
}

void AsmMethodPass::visitCall(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  if (bb == nullptr)
    return;

  ir_node *address = get_Call_ptr(node);
  ir_type *callType = get_Call_type(node);
  assert(is_Address(address));
  ir_entity *entity = get_Address_entity(address);
  auto funcName = std::string(get_entity_name(entity));
  int nParams = get_Call_n_params(node);
  int addSize = 0;

  if (funcName == "print_int" ||
      funcName == "write_int") {
    assert(nParams == 1);
    ir_node *p = get_Call_param(node, 0);

    bb->pushInstr(Asm::Mov, getNodeOp(p), Asm::Op(Asm::RegName::di, Asm::RegMode::R));
  } else if (funcName == "allocate") {
    assert(nParams == 2);
    ir_node *n = get_Call_param(node, 0);
    ir_node *size = get_Call_param(node, 1);

    // num in rdi
    bb->pushInstr(Asm::Mov, getNodeOp(n), Asm::Op(Asm::RegName::di, Asm::getRegMode(n)));
    // size in rsi
    bb->pushInstr(Asm::Mov, getNodeOp(size), Asm::Op(Asm::RegName::si, Asm::getRegMode(size)));
  } else {
    // Normal MiniJava functions
    addSize = nParams * 8;

    if (addSize > 0) {
      bb->pushInstr(Asm::Sub, Asm::Op(addSize), Asm::rsp());
    }

    int offset = 0;
    for (int i = 0; i < nParams; i ++) {
      auto paramOp = getNodeOp(get_Call_param(node, i));
      if (paramOp.type != Asm::OP_IMM) {
        auto regOp = Asm::Op(Asm::RegName::cx, Asm::RegMode::R);
        bb->pushInstr(Asm::Mov, paramOp, regOp);
        paramOp = regOp;
      }

      bb->pushInstr(Asm::Movq, paramOp, Asm::Op(Asm::rsp(), offset));

      offset += 8;
    }
  }

  bb->pushInstr(Asm::Call, std::move(funcName));


  if (addSize > 0) {
    bb->pushInstr(Asm::Add, Asm::Op(addSize), Asm::rsp());
  }

  // Write result of function call into stack slot of call->proj->proj node
  if (get_method_n_ress(callType) > 0) {
    assert(get_method_n_ress(callType) == 1);

    ir_node *projSucc = getSucc(node, iro_Proj, mode_T);
      if (projSucc != nullptr) {
      ir_node *resultProj = getProjSucc(projSucc);

      if (resultProj != nullptr) {
        bb->pushInstr(Asm::Mov, Asm::rax(), getNodeOp(resultProj));
      }
    }
  }
}

void AsmMethodPass::visitCmp(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  if (bb == nullptr)
    return;

  ir_node *leftNode = get_Cmp_left(node);
  ir_node *rightNode = get_Cmp_right(node);

  // For simplicity, we ignore the case where the left node is a constant.

  auto leftRegMode = Asm::getRegMode(leftNode);
  bb->pushJumpInstr(Asm::makeMov(leftRegMode,
                                 getNodeOp(leftNode),
                                 Asm::Op(Asm::RegName::bx, leftRegMode)));

  // Left and right swapped!
  bb->pushJumpInstr(Asm::Cmp,
                    getNodeOp(rightNode),
                    Asm::Op(Asm::RegName::bx, leftRegMode),
                    nodeStr(node));
}

void AsmMethodPass::visitCond(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  if (bb == nullptr)
    return;

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
    // Just jump to that block, and the Phi node (if it exists) will do the je/jne instructions
    bb->pushJumpInstr(Asm::makeJump(Asm::getBlockLabel(trueBlock), ir_relation_true));
  } else {
    // Control flow comparison, jump to the appropriate basic block
    bb->pushJumpInstr(Asm::makeJump(Asm::getBlockLabel(trueBlock), relation));
    bb->pushJumpInstr(Asm::makeJump(Asm::getBlockLabel(falseBlock), getInverseRelation(relation)));
  }
}

void AsmMethodPass::visitJmp(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  if (bb == nullptr)
    return;

  ir_node *jumpTarget = getNthSucc(node, 0);
  assert(is_Block(jumpTarget));

  bb->pushJumpInstr(Asm::makeJump(Asm::getBlockLabel(jumpTarget), ir_relation_true));
}

void AsmMethodPass::visitEnd(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);

  if (bb == nullptr)
    return; // Unreachable
}


void AsmMethodPass::visitReturn(ir_node *node) {
  auto bb = getBB(node);

  if (bb == nullptr) {
    // Happens for unreachable stataments
    return;
  }

  ir_node *succ = getNthSucc(node, 0);
  assert(is_Block(succ));

  if (get_Return_n_ress(node) > 0) {
    ir_node *opNode = get_Return_res(node, 0);

    auto op = getNodeOp(opNode);
    bb->pushInstr(Asm::Instr(Asm::Mov, op, Asm::rax()));
  }

  // Jump to end block
  // return nodes should have exactly one successor, the end block.
  bb->pushJumpInstr(Asm::makeJump(Asm::getBlockLabel(succ), ir_relation_true));
}

void AsmMethodPass::visitLoad(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  if (bb == nullptr)
    return;

  ir_node *pred = get_Load_ptr(node);
  ir_node *succ;

  // Load nodes have 2 successors, one Proj M and another one, which we want.
  succ = getNthSucc(node, 0);
  if (get_irn_mode(succ) == mode_M)
    succ = getNthSucc(node, 1);

  if (succ == nullptr) {
    std::cout << nodeStr(node) << " has no successor" << std::endl;
    // We load something but don't end up using it at all?
    return;
  }

  assert(get_irn_mode(succ) != mode_M); // ! Load nodes have 2 successor Proj nodes

  /*
   * 1) Load contents of pred slot into temporary register. This now contains the address to read from
   * 2) Write the value at that address into a second temporary register
   * 3) Write that value to succ's slot
   */
  auto predRegMode = Asm::getRegMode(pred);
  auto succRegMode = Asm::getRegMode(succ);

  // 1)
  bb->pushInstr(Asm::makeMov(Asm::getRegMode(pred), getNodeOp(pred),
                             Asm::Op(Asm::RegName::bx, predRegMode), "1)"));
  // 2)
  bb->pushInstr(Asm::makeMov(Asm::getRegMode(succ),
                             Asm::Op(Asm::RegName::bx, predRegMode, 0),
                             Asm::Op(Asm::RegName::cx, succRegMode), "2)"));
  // 3)
  bb->pushInstr(Asm::Movq, Asm::rcx(), getNodeOp(succ), "3)");
}

void AsmMethodPass::visitStore(ir_node *node) {
  PRINT_ORDER;
  auto bb = getBB(node);
  if (bb == nullptr)
    return;

  ir_node *source = get_Store_value(node);
  ir_node *dest = get_Store_ptr(node);

  // dest is a pointer, so we need to save into the address of that pointer,
  // not into the register of that node.
  assert(get_irn_mode(dest) == mode_P);

  /*
   * 1) Copy contents from DEST register into tmp register (will contain address)
   * 2) write SOURCE value into address in tmp register.
   */

  bb->pushInstr(Asm::makeMov(Asm::getRegMode(dest), getNodeOp(dest), Asm::rbx(), "1)"));

  // rb now contains the address to write to!
  auto sourceOp = getNodeOp(source);
  Asm::Op tmpOp;
  if (sourceOp.type == Asm::OP_IMM) {
    tmpOp = sourceOp;
  } else {
    tmpOp = Asm::Op(Asm::RegName::cx, Asm::getRegMode(dest));
    bb->pushInstr(Asm::makeMov(Asm::getRegMode(dest),
                               sourceOp,
                               tmpOp,
                               "2)"));
    tmpOp = Asm::Op(tmpOp.reg.name, Asm::getRegMode(source)); // Different mode!
  }

  bb->pushInstr(Asm::makeMov(Asm::getRegMode(source), tmpOp, Asm::Op(Asm::rbx(), 0), "3)"));
}

void AsmMethodPass::visitPhi(ir_node *node) {
  PRINT_ORDER;
  if (get_Phi_loop(node))
    return; // ???

  auto bb = getBB(node);
  if (bb == nullptr)
    return;

  bool isSwap = false;
  for (int i = 0; i < get_Phi_n_preds(node); i ++) {
    ir_node *pred = get_Phi_pred(node, i);
    //std::cout << "   " << nodeStr(pred) << std::endl;
    if (is_Phi(pred)) {
      for (int x = 0; x < get_Phi_n_preds(pred); x ++) {
        if (get_Phi_pred(pred, x) == node &&
            get_nodes_block(node) == get_nodes_block(pred)) {
          isSwap = true;
          goto swapCheck;
        }
      }
    }
  }
  swapCheck:

  int nPreds = get_Phi_n_preds(node);
  assert(nPreds == 2);
  bool needsLabels = get_nodes_block(get_Block_cfgpred(get_nodes_block(node), 0)) ==
                     get_nodes_block(get_Block_cfgpred(get_nodes_block(node), 1));

  if (isSwap) {
    //std::cout << "SwapPhi: " << nodeStr(node) << std::endl;
    generateSwapPhi(node);
  } else {
    if (needsLabels) {
      //std::cout << "NeedsLabels: " << nodeStr(node) << std::endl;
      generateBoolPhi(node);
    } else {
      //std::cout << "Normal: " << nodeStr(node) << std::endl;
      generateNormalPhi(node);
    }
  }

#if 0
  std::cout << nodeStr(node) << " in block " << nodeStr(get_nodes_block(node)) << ":" << std::endl;
  for (int i = 0; i < get_Phi_n_preds(node); i ++) {
    ir_node *pred = get_Phi_pred(node, i);
    //ir_node *cfgPred = get_Block_cfgpred(get_nodes_block(node), i);
    std::cout << "    Pred: " << nodeStr(pred) << ", Block: " << nodeStr(get_nodes_block(pred))
              << ", PhiPredBlock: " << nodeStr(get_nodes_block(get_Block_cfgpred(get_nodes_block(node), i)))
              << std::endl;

  }
#endif
}

void AsmMethodPass::generateBoolPhi(ir_node *node) {
  auto bb = getBB(node);
  // Phi node used for bool values, with both preds in the same block.
  ir_node *truePred = get_Phi_pred(node, 0);
  ir_node *trueProj = get_Block_cfgpred(get_nodes_block(node), 0);
  ir_node *falsePred = get_Phi_pred(node, 1);
  ir_node *falseProj = get_Block_cfgpred(get_nodes_block(node), 1);

  assert(is_Proj(trueProj));
  assert(is_Proj(falseProj));

  if (get_Proj_num(falseProj) != pn_Cond_false) {
    assert(get_Proj_num(trueProj) == pn_Cond_false);
    std::swap(falseProj, trueProj);
    std::swap(falsePred, truePred);
  }
  assert(get_Proj_num(trueProj) == pn_Cond_true);
  assert(get_Proj_num(falseProj) == pn_Cond_false);
  ir_node *condNode = get_Proj_pred(trueProj);
  assert(is_Cond(condNode));
  ir_node *selector = get_Cond_selector(condNode);
  assert(is_Cmp(selector));
  ir_relation relation = get_Cmp_relation(selector);
  std::string falseLabel = "false_" + std::to_string(get_irn_node_nr(node));
  std::string phiLabel = "phi_" + std::to_string(get_irn_node_nr(node));

  // TODO: The code duplication here isn't exactly nice
  bb->pushStartPhiInstr(Asm::makeJump(falseLabel, getInverseRelation(relation)));
  // The cmp instruction was already written by visitCmp!
  {
    // Write this phiPred into the stack slot of the phi node
    auto srcOp = getNodeOp(truePred);

    if (srcOp.type != Asm::OP_IMM) {
      auto tmpReg = Asm::Op(Asm::RegName::r15, Asm::RegMode::R);
      bb->pushStartPhiInstr(Asm::Movq, srcOp, tmpReg, "phi tmp 3");
      srcOp = tmpReg;
    }

    bb->pushStartPhiInstr(Asm::Movq,
                          srcOp,
                          Asm::Op(Asm::rbp(), ssm.getStackSlot(node)),
                          "phi dst");
    // end of true case, jump to phi label
    bb->pushStartPhiInstr(Asm::makeJump(phiLabel, ir_relation_true));
  }

  // False case
  {
    bb->pushStartPhiInstr(Asm::Label, falseLabel);
    auto srcOp = getNodeOp(falsePred);

    if (srcOp.type != Asm::OP_IMM) {
      auto tmpReg = Asm::Op(Asm::RegName::r15, Asm::RegMode::R);
      bb->pushStartPhiInstr(Asm::Movq, srcOp, tmpReg, "phi tmp 4");
      srcOp = tmpReg;
    }

    bb->pushStartPhiInstr(Asm::Movq,
                          srcOp,
                          Asm::Op(Asm::rbp(), ssm.getStackSlot(node)),
                          "phi dst");
  }

  // end phi
  bb->pushStartPhiInstr(Asm::Label, phiLabel);
}

void AsmMethodPass::generateSwapPhi(ir_node *node) {
  auto bb = getBB(node);
  // Write this node's current value into its tmp slot
  {
    auto srcOp = getNodeOp(node);
    auto tmpReg = Asm::Op(Asm::RegName::r15, Asm::RegMode::R);
    bb->pushStartPhiInstr(Asm::Movq, srcOp, tmpReg);

    bb->pushStartPhiInstr(Asm::Movq, tmpReg,
                          Asm::Op(Asm::rbp(), ssm.getTmpSlot(node)),
                          "swap phi old val " + nodeStr(node));
  }

  ir_node *phiPred = get_Phi_pred(node, 0);
  ir_node *phiProj = get_Block_cfgpred(get_nodes_block(node), 0);
  ir_node *otherPred = get_Phi_pred(node, 1);
  ir_node *otherProj = get_Block_cfgpred(get_nodes_block(node), 1);

  //std::cout << "phiProj: " << nodeStr(phiProj) << std::endl;
  //std::cout << "otherProj: " << nodeStr(otherProj) << std::endl;

  if (!is_Phi(phiPred)) {
    std::swap(phiPred, otherPred);
    std::swap(phiProj, otherProj);
  }
  assert(is_Phi(phiPred));
  assert(!is_Phi(otherPred));

  // Phi pred. Read from its tmp slot to get the value before the phi evaluation,
  // write into the stack slot of this node
  {
    auto predBB = getBB(phiProj);

    assert(getBB(phiPred) == getBB(node));
    // tmp register
    auto tmpReg = Asm::Op(Asm::RegName::r15, Asm::RegMode::R);
    predBB->pushPhiInstr(Asm::Movq,
                         Asm::Op(Asm::rbp(), ssm.getTmpSlot(phiPred)),
                         tmpReg);

    // Now from tmp register into our stack slot
    predBB->pushPhiInstr(Asm::Movq,
                         tmpReg, Asm::Op(Asm::rbp(), ssm.getStackSlot(node)));

  }

  // Not a phi pred. Write into its basic block,
  // once into the stack slot and once into the tmp slot!
  {
    assert(getBB(otherPred) != getBB(node));
    auto predBB = getBB(otherPred);

    assert(predBB != bb);
    /* Control flow, generate phi instructions in the predecessor basic blocks */
    // Write this phiPred into the stack slot of the phi node
    auto tmpReg = Asm::Op(Asm::RegName::r15, Asm::RegMode::R);
    predBB->pushPhiInstr(Asm::Movq, getNodeOp(otherPred), tmpReg, "phi tmp 2");

    predBB->pushPhiInstr(Asm::Movq, tmpReg,
                         Asm::Op(Asm::rbp(), ssm.getStackSlot(node)),
                         nodeStr(node) + " commit");
  }
}


void AsmMethodPass::generateNormalPhi(ir_node *node) {
  auto bb = getBB(node);

  int nPreds = get_Phi_n_preds(node);
  assert(nPreds == 2);

  for (int i = 0; i < nPreds; i ++) {
    ir_node *phiPred = get_Phi_pred(node, i);
    ir_node *blockPredNode = get_Block_cfgpred(get_nodes_block(node), i);

    auto predBB = getBB(blockPredNode);
    assert(predBB != bb);
    /* Control flow, generate phi instructions in the predecessor basic blocks */
    // Write this phiPred into the stack slot of the phi node

    Asm::Op srcOp = getNodeOp(phiPred);
    if (srcOp.type != Asm::OP_IMM) {
      auto tmpReg = Asm::Op(Asm::RegName::r15, Asm::RegMode::R);
      predBB->pushPhiInstr(Asm::Movq,
                           srcOp, tmpReg);
      srcOp = tmpReg;
    }
    predBB->pushPhiInstr(Asm::Movq,
                         srcOp,
                         Asm::Op(Asm::rbp(), ssm.getStackSlot(node)));
  }

}

void AsmMethodPass::visitMinus(ir_node *node) {
  PRINT_ORDER;
  ir_node *sourceNode = get_Minus_op(node);
  auto bb = getBB(node);

  auto regMode = Asm::getRegMode(node);
  bb->pushInstr(Asm::Mov, getNodeOp(sourceNode), Asm::Op(Asm::RegName::bx, regMode));

  bb->pushInstr(Asm::Neg, Asm::Op(Asm::RegName::bx, regMode));
  bb->pushInstr(Asm::makeMov(regMode,
                             Asm::Op(Asm::RegName::bx, regMode),
                             getNodeOp(node)));
}
