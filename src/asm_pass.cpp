#include "asm_pass.hpp"

#include <sstream>

void AsmPass::before() {
  //writer.writeTextSection();
}

void AsmPass::visitMethod(ir_graph *graph) {
#if 0
  const char *graphName = get_entity_ld_name(get_irg_entity(graph));
  std::stringstream ss;

  ss << "Begin " << graphName;
  writer.writeComment(ss.str());

  writer.writeInstruction(".p2align 4,,15");
  ss.str("");
  ss << ".globl " << graphName;
  writer.writeInstruction(ss.str());

  ss.str("");
  ss << ".type " << graphName << ", @function";
  writer.writeInstruction(ss.str());

  ss.str("");
  ss << graphName << ":";
  writer.writeLabel(ss.str());
#endif

  auto func = new AsmFunction();
  AsmMethodPass methodPass(graph, func);
  methodPass.run();
  functions.push_back(func);



#if 0
  ss.str("");
  ss << ".size " << graphName << ", .-" << graphName;
  writer.writeInstruction(ss.str());

  ss.str("");
  ss << "End " << graphName;
  writer.writeComment(ss.str());
#endif
}

void AsmMethodPass::before() {
  
}
