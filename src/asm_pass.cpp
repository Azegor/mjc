#include "asm_pass.hpp"

#include <fstream>

void AsmPass::before() {
  //writer.writeTextSection();
}

void AsmPass::visitMethod(ir_graph *graph) {
  const char *functionName = get_entity_ld_name(get_irg_entity(graph));
  Asm::Function func(functionName);
  AsmMethodPass methodPass(graph, &func);
  methodPass.run();
  asmProgram.addFunction(std::move(func));
}

void AsmPass::after()
{
  std::ofstream outputFile(outputFileName);
  if (!outputFile.is_open()) {
    throw std::runtime_error("could not open file '" + outputFileName + '\'');
  }
  outputFile << asmProgram << std::endl;
}
