#include "asm_pass.hpp"

#include <fstream>

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

void AsmMethodPass::visitAdd(ir_node *add) {
  // 1. get left and right predecessor of add node
  // 2. get values from predecessor. Either immediate or generated val (stack slot)
  // 3. create memory or immediate operands
  // 4. create instruction with operands

  // TODO cleanup and avoid code duplication in other visit Methods
  auto regMode = Asm::X86Reg::getRegMode(get_irn_mode(add));
  auto leftOp = getNodeResAsInstOperand(get_Add_left(add));
  Asm::X86Reg leftReg(Asm::X86Reg::Name::ax, regMode);
  auto leftRegInst = loadToReg(std::move(leftOp), leftReg);
  currentBB->addInstruction(std::move(leftRegInst));
  auto rightOp = getNodeResAsInstOperand(get_Add_right(add));
  Asm::X86Reg rightReg(Asm::X86Reg::Name::bx, regMode);
  auto rightRegInst = loadToReg(std::move(rightOp), rightReg);
  currentBB->addInstruction(std::move(rightRegInst));
  //   if (auto rWriteOp = dynamic_cast<Asm::WritableOperand *>(rightOp.get())) {
  currentBB->emplaceInstruction<Asm::Add>(Asm::Register::get(leftReg), Asm::Register::get(rightReg),
                                          "Node " + std::to_string(get_irn_node_nr(add)));
  currentBB->addInstruction(writeResToStackSlot(rightReg, add));
}
