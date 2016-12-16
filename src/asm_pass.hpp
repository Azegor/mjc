#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include <unordered_map>
#include <vector>

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


class StackSlotManager {
  int currentOffset = 0;
  std::unordered_map<ir_node *, int> offsets;
public:
  StackSlotManager() {}

  int getStackSlot(ir_node *node) {
    auto pos = offsets.find(node);
    if (pos == offsets.end()) {
      return offsets.insert({node, (currentOffset -= 8)}).second;
    }
    return pos->second;
  }
};

class AsmMethodPass : public FunctionPass<AsmMethodPass, Asm::Instruction> {
  StackSlotManager ssm;

public:
  AsmMethodPass(ir_graph *graph, Asm::Function *func) : FunctionPass(graph), func(func) {}

  void before() { std::cout << "### visiting function " << get_entity_ld_name(get_irg_entity(graph)) << std::endl; }
  void after() { std::cout << "### finished function " << get_entity_ld_name(get_irg_entity(graph)) << std::endl; }

  void initBlock(ir_node *b) {
    blockNodesList.insert({b, std::vector<ir_node *>()}); // init empty vector
    enqueue(b);
  }

  void visitBlock(ir_node *b) {
    this->currentBB = func->newBB("Block " + std::to_string(get_irn_node_nr(b)));
    std::cout << " --- visiting " << currentBB->getComment() << std::endl;

    auto &blockNodes = blockNodesList.at(b);
    for (auto n : blockNodes) {
      assert(!is_Block(n));
      visitNode(n);
    }

    this->currentBB = nullptr; // to be safe
  }

  void defaultInitOp(ir_node *n) {
    blockNodesList[get_nodes_block(n)].push_back(n);
  }

  void defaultVisitOp(ir_node *n) {
    ir_printf("  visiting node %n (%N) in bb %s\n", n, n, currentBB->getComment().c_str());
  }


  void visitAdd(ir_node *add) {
    (void) add;
    // 1. get left and right predecessor of add node
    // 2. get values from predecessor. Either immediate or generated val (stack slot)
    // 3. create memory or immediate operands
    // 4. create instruction with operands
    // TODO (only example)
    /*auto instr = */currentBB->emplaceInstruction<Asm::Add>(
        getNodeInstrOperand(get_Add_left(add)),
        getNodeInstrOperand(get_Add_right(add)),
        "Node " + std::to_string(get_irn_node_nr(add)));
//     setVal(add, instr);
  }

  Asm::OperandPtr getNodeInstrOperand(ir_node *node) {
    switch(get_irn_opcode(node)) {
      case iro_Const:
        return std::make_unique<Asm::Immediate>(get_Const_tarval(node));
      default:
        return std::make_unique<Asm::Memory>(ssm.getStackSlot(node),
            Asm::X86_64Register(Asm::X86_64Register::Name::rbp,
                                Asm::X86_64Register::getRegMode(get_irn_mode(node)))
            );
    }
  }

private:
  std::unordered_map<ir_node *, std::vector<ir_node *>> blockNodesList;
  Asm::Function *func;

  Asm::BasicBlock *currentBB;
};

#endif

