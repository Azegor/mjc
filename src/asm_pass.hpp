#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include <unordered_map>
#include <vector>

#include "asm.hpp"
#include "firm_pass.hpp"

class AsmPass : public ProgramPass<AsmPass> {
  const std::string &outputFileName;

public:
  AsmPass(std::vector<ir_graph *> &graphs, const std::string &outputFileName)
      : ProgramPass(graphs), outputFileName(outputFileName) {}

  void before();
  void visitMethod(ir_graph *graph);
  void after();

private:
  Asm::Programm asmProgram;
};

class StackSlotManager {
  int32_t currentOffset = 0;
  std::unordered_map<ir_node *, int32_t> offsets;

public:
  StackSlotManager() {}

  int32_t getStackSlot(ir_node *node) {
    auto pos = offsets.find(node);
    if (pos == offsets.end()) {
      pos = offsets.insert({node, (currentOffset += 8)}).first;
    }
    return pos->second;
  }

  int32_t getLocVarUsedSize() const { return currentOffset; }
};

class AsmMethodPass : public FunctionPass<AsmMethodPass, Asm::Instruction> {
  StackSlotManager ssm;

public:
  AsmMethodPass(ir_graph *graph, Asm::Function *func) : FunctionPass(graph), func(func) {}

  void before() {
    std::cout << "### visiting function " << get_entity_ld_name(get_irg_entity(graph)) << std::endl;
  }
  void after() {
    std::cout << "### finished function " << get_entity_ld_name(get_irg_entity(graph)) << std::endl;
    func->setARSize(ssm.getLocVarUsedSize());
  }

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

  void defaultInitOp(ir_node *n) { blockNodesList[get_nodes_block(n)].push_back(n); }

  void defaultVisitOp(ir_node *n) {
    ir_printf("  visiting node %n (%N) in bb %s\n", n, n, currentBB->getComment().c_str());
  }

  void visitAdd(ir_node *add);

  Asm::OperandPtr getNodeResAsInstOperand(ir_node *node) {
    switch (get_irn_opcode(node)) {
    case iro_Const:
      return std::make_unique<Asm::Immediate>(get_Const_tarval(node));
    default:
      return std::make_unique<Asm::MemoryBase>(
          ssm.getStackSlot(node),
          Asm::X86Reg(Asm::X86Reg::Name::bp,
                              Asm::X86Reg::getRegMode(get_irn_mode(node))));
    }
    __builtin_trap();
  }

  Asm::InstrPtr loadToReg(Asm::OperandPtr val, Asm::X86Reg reg) {
    return std::make_unique<Asm::Mov>(std::move(val), Asm::Register::get(reg),
                                      "load stackslot val to reg");
  }

  Asm::InstrPtr writeResToStackSlot(Asm::X86Reg reg, ir_node *node) {
    return std::make_unique<Asm::Mov>(
        Asm::Register::get(reg),
        std::make_unique<Asm::MemoryBase>(
            ssm.getStackSlot(node),
            Asm::X86Reg(Asm::X86Reg::Name::bp,
                                Asm::X86Reg::getRegMode(get_irn_mode(node)))),
        "store reg to stackslot");
  }

private:
  std::unordered_map<ir_node *, std::vector<ir_node *>> blockNodesList;
  Asm::Function *func;

  Asm::BasicBlock *currentBB;
};

#endif
