#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include "firm_pass.hpp"
#include "asm_writer.hpp"

#include <unordered_map>

namespace assembly {
class Instruction {

};

};

using InstrList = std::vector<assembly::Instruction>;

class AsmFunction {
public:
  ir_graph *graph;
  std::vector<InstrList> blockInstructions;
};

class AsmPass : public ProgramPass<AsmPass> {
public:
  AsmPass(std::vector<ir_graph *> &graphs) : ProgramPass(graphs) {}

  void before() override;
  void visitMethod(ir_graph *graph) override;

private:
  std::vector<AsmFunction*> functions;
};

class AsmMethodPass : public FunctionPass<AsmMethodPass> {
public:
  AsmMethodPass(ir_graph *graph, AsmFunction *func) : FunctionPass(graph), func(func) {}

  void before();

  void visitBlock(ir_node *b) {
    func->blockInstructions.emplace_back();
    auto instrList = &func->blockInstructions[func->blockInstructions.size() - 1];
    this->currentInstrList = instrList;

    std::cout << "Block" << std::endl;
  }

private:
  AsmFunction *func;
  InstrList *currentInstrList;

  //std::unordered_map<ir_node *, std::vector<assembly::Instruction>> blockMap;
};

#endif

