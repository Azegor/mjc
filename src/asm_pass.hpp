#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include "firm_pass.hpp"
#include "asm_writer.hpp"

class AsmPass : public ProgramPass<AsmPass> {
public:
  AsmPass(std::vector<ir_graph *> &graphs, std::ostream &out) : ProgramPass(graphs), writer(out) {}

  void before() override;
  void visitMethod(ir_graph *graph) override;

private:
  AsmWriter writer;
};

#endif

