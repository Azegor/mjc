#include "asm_direct_pass.hpp"

#include <iostream>
#include <fstream>
#include <cstring>

void AsmDirectPass::visitMethod(ir_graph *graph) {
  auto funcName = get_entity_ld_name(get_irg_entity(graph));
  std::cout << "\nGraph: " << graph << std::endl;
  println("\t.p2align 4,,15");
  println("\t.globl main");
  ss << "\t.type " << funcName << ", @function\n";
  ss << funcName << ":\n";

  // Do stuff
  DirectFunctionPass funcPass(graph, ss);
  funcPass.run();

  ss << "\t.size " << funcName << ", .-" << funcName << "\n";
  ss << "# ------------------ End " << funcName << " ---------------------\n\n\n";
}

void AsmDirectPass::after() {
  std::ofstream outputFile(outputFileName);
  if (!outputFile.is_open()) {
    throw std::runtime_error("could not open file '" + outputFileName + '\'');
  }
  outputFile << ss.str() << std::endl;
}


// ----------------------------------------------------------------------------------------------

// rbp = base pointer
// rsp = stack pointer
// rbx, rcx, rdx, rsi, rdi = general purpose registers
// rax = return value
// We use rbx and rcx for values of arithmetic operations

void DirectFunctionPass::after() {
  for (ir_node *block : blockSchedule) {
    auto &commands = blockMap[block];
    std::cout << "Writing Block " << get_irn_node_nr(block) << ":" << std::endl;
    for (const std::string &cmd : commands) {
      std::cout << cmd<< std::endl;
      this->ss << cmd << "\n";
    }
  }
}

void DirectFunctionPass::visitStart(ir_node *startNode) {
  std::cout << __FUNCTION__ << " " << get_irn_node_nr (startNode) << std::endl;
  auto cmds = getCommands(startNode);
  cmds->push_back("\tpushq %rbp");
  cmds->push_back("\tmov %rsp, %rbp");
}

void DirectFunctionPass::visitReturn(ir_node *returnNode) {
  std::cout << __FUNCTION__ << " " << get_irn_node_nr (returnNode) << std::endl;
  auto cmds = getCommands(returnNode);
  // The operand has just been visited so its result is in rbx
  // TODO: Constants...
  cmds->push_back("\tmov %rbx, %rax");
  cmds->push_back("\tleave");
  cmds->push_back("\tret");
}

void DirectFunctionPass::visitConst(ir_node *constNode) {
  std::cout << __FUNCTION__ << " " << get_irn_node_nr (constNode) << std::endl;
  auto cmds = getCommands(constNode);
  auto reg = getRegister();
  std::stringstream ss;
  ss << "\tmov $" << get_tarval_long(get_Const_tarval(constNode)) << ", "
     << "%" << reg->name;

  cmds->push_back(ss.str());
}

void DirectFunctionPass::visitEnd(ir_node *endNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (endNode) << std::endl;
}

void DirectFunctionPass::visitBlock(ir_node *blockNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (blockNode) << std::endl;
  auto cmds = getCommands(blockNode);
  cmds->push_back("." + std::string(getBlockLabel(blockNode)) + ":");
}

void DirectFunctionPass::visitCall(ir_node *callNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (callNode) << std::endl;
  auto cmds = getCommands(callNode);
  ir_entity *callee = get_Call_callee (callNode);
  if (strcmp ("print_int", get_entity_ld_name(callee)) == 0) {
    std::stringstream ss;
      ss << "\tmov ";
    if (is_Const(get_Call_param(callNode, 0))) {
      ss << "$" << get_tarval_long(get_Const_tarval(get_Call_param(callNode, 0)));
    } else {
      assert(false);
    }
    ss << ", %rdi";

    cmds->push_back(ss.str());
    cmds->push_back("\tcall " + std::string(get_entity_ld_name(callee)));

    // Don't need the value of rbx anymore
    registers[0].free = true;
  } else if (strcmp ("read_int", get_entity_ld_name(callee)) == 0) {
    auto reg = getRegister();
    cmds->push_back("\tcall read_int");
    cmds->push_back("\tmov %rax, %" + std::string(reg->name));
  } else {
    assert (0);
  }
}

void DirectFunctionPass::visitAdd(ir_node *addNode) {
  std::cout << __FUNCTION__ << " " << get_irn_node_nr (addNode) << std::endl;
  auto cmds = getCommands(addNode);
  // Operands are in rbx and rcx
  std::stringstream ss;
  ss << "\tadd %" << registers[1].name << ", %" << registers[0].name;
  cmds->push_back(ss.str());
  registers[1].free = true; // Frees rcx
}

void DirectFunctionPass::visitMul(ir_node *mulNode) {
  auto cmds = getCommands(mulNode);
  std::cout << __FUNCTION__ << " " << get_irn_node_nr (mulNode) << std::endl;
  // Operands are in rbx and rcx
  std::stringstream ss;
  ss << "\timul %" << registers[1].name << ", %" << registers[0].name;
  cmds->push_back(ss.str());
  registers[1].free = true; // Frees rcx
}

void DirectFunctionPass::visitJmp(ir_node *jmpNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (jmpNode) << std::endl;
}

void DirectFunctionPass::visitPhi(ir_node *phiNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (phiNode) << std::endl;
}

void DirectFunctionPass::visitCond(ir_node *condNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (condNode) << std::endl;
}

void DirectFunctionPass::visitCmp(ir_node *cmpNode) {
  //std::cout << __FUNCTION__ << " " << get_irn_node_nr (cmpNode) << std::endl;
  auto cmds = getCommands(cmpNode);
  std::stringstream ss;
  ss << "\tcmp ";
  //ss << "\tcmp %" << registers[0].name << ", ";//%" << registers[1].name;
  if (is_Const(get_Cmp_right(cmpNode))) {
    ir_node *c = get_Cmp_right(cmpNode);
    ss << "$" << get_tarval_long(get_Const_tarval(c));
  } else assert(false);

  ss << ", %rbx";

  cmds->push_back(ss.str());
  //cmds->push_back("jmp .L73");
  registers[0].free = true;
  registers[1].free = true;
}

void DirectFunctionPass::visitProj(ir_node *projNode) {
  std::cout << __FUNCTION__ << " " << get_irn_node_nr (projNode) << std::endl;
  auto cmds = getCommands(projNode);
  if (projNode == get_irg_args(this->graph)) {
    cmds->push_back("\tmov $1337, " + std::string(getRegister()->name));
  } else {
    ir_node *pred = get_Proj_pred(projNode);
    if (is_Cond(pred) && get_irn_mode(projNode) == mode_X) {
      // Create a jump
      //std::cout << "Cond pred" << std::endl;
    }
  }
}
