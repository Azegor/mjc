#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include "firm_pass.hpp"
#include "asm.hpp"

class AsmPass : public ProgramPass<AsmPass> {
public:
  AsmPass(std::vector<ir_graph *> &graphs) : ProgramPass(graphs) {}

  void before();
  void visitMethod(ir_graph *graph);

private:
  Asm::Programm asmProgram;
};



class AsmMethodPass : public FunctionPass<AsmMethodPass> {
public:
  AsmMethodPass(ir_graph *graph, Asm::Function *func) : FunctionPass(graph), func(func) {}

  void before();

  void visitBlock(ir_node *b) {
    (void)b;
    Asm::BasicBlock bb;
    this->currentBB = &bb;



    std::cout << "Block" << std::endl;
  }

private:
  Asm::Function *func;

  Asm::BasicBlock *currentBB;
};

#endif

