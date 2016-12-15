#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include "firm_pass.hpp"
#include "asm.hpp"

class AsmPass : public ProgramPass<AsmPass> {
  const std::string &outputFileName;
public:
  AsmPass(std::vector<ir_graph *> &graphs, const std::string &outputFileName) : ProgramPass(graphs), outputFileName(outputFileName) {}

  void before();
  void visitMethod(ir_graph *graph);
  void after();

private:
  Asm::Programm asmProgram;
};



class AsmMethodPass : public FunctionPass<AsmMethodPass> {
public:
  AsmMethodPass(ir_graph *graph, Asm::Function *func) : FunctionPass(graph), func(func) {}

  void before();

  void initBlockNodesTopological() {
    inc_irg_visited(graph);
    init_blocks_topological(get_irg_end(graph));
  }

  void init_blocks_topological(ir_node *irn)
  {
    if (irn_visited(irn)) {
      return;
    }

    mark_irn_visited(irn);

    assert(!is_Block(irn)); // irn is never a block, since predecessors of blocks are X nodes
    ir_node *const block = get_nodes_block(irn);
    for (int i = 0; i < get_irn_arity(block); ++i) {
      ir_node *const pred = get_irn_n(block, i);
      init_blocks_topological(pred);
    }
//     ir_printf("coming from %n(%N), enqueuing %n (%N)\n", irn, irn, block, block);
    initBlock(block);
    enqueue(block);
  }

  void initWorkQueue() {
    initBlockNodesTopological();
  }

  void visitBlock(ir_node *b) {
    (void)b;
    Asm::BasicBlock bb;
    this->currentBB = &bb;



    std::cout << "Block" << std::endl;
    func->addBB(std::move(bb));
  }

private:
  Asm::Function *func;

  Asm::BasicBlock *currentBB;
};

#endif

