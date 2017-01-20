#ifndef ASM_PASS_HPP
#define ASM_PASS_HPP

#include <unordered_map>
#include <vector>

#include "asm.hpp"
#include "firm_pass.hpp"

//#define ORDER
//#define STACK_SLOTS


#ifdef ORDER
#define PRINT_ORDER ir_printf("%s: %n %N\n", __FUNCTION__, node, node);
#else
#define PRINT_ORDER (void)node;
#endif

std::string nodeStr(ir_node *node);

class AsmPass : public ProgramPass<AsmPass> {

public:
  AsmPass(std::vector<ir_graph *> &graphs)
      : ProgramPass(graphs) {}

  void before();
  void visitMethod(ir_graph *graph);

  Asm::Program *getProgram() { return &asmProgram; }

private:
  Asm::Program asmProgram;
};

class StackSlotManager {
  int32_t currentOffset = 8;
  std::unordered_map<ir_node *, int32_t> offsets;
  std::unordered_map<ir_node *, int32_t> tmpSlots;

public:
  StackSlotManager() {}

  int32_t getStackSlot(ir_node *node, Asm::BasicBlock *bb) {
    assert(!is_Block(node));

    auto pos = offsets.find(node);
    if (pos == offsets.end()) {
      pos = offsets.insert({node, -currentOffset}).first;
#ifdef STACK_SLOTS
      ir_printf("New Stack slot for node %n %N: %d\n", node, node, pos->second);
#endif
      (void)bb; // TODO: Remove bb param

      currentOffset += 8;
    }
    return pos->second;
  }

  bool hasSlot(ir_node *node) {
    return offsets.find(node) != offsets.end();
  }

  void copySlot(ir_node *from, ir_node *to) {
    assert(!hasSlot(to));
    int offset;
    if (!hasSlot(from)) {
      offset = getStackSlot(from, nullptr);
    } else {
      offset = offsets[from];
    }

#ifdef STACK_SLOTS
    std::cout << "New stack slot for " << nodeStr(to) << ": "
              << offset << " (copied from " << nodeStr(from) << ")" << std::endl;
#endif

    offsets.insert({to, offset});
  }

  // For parameters
  void setSlot(ir_node *node, int32_t offset) {
    assert(!hasSlot(node));

#ifdef STACK_SLOTS
    std::cout << "New stack slot for " << nodeStr(node) << ": " << offset << std::endl;
#endif

    offsets.insert({node, offset});
  }


  // Used to resolve swaps with phi nodes
  int getTmpSlot(ir_node *node) {
    assert(is_Phi(node));

    auto pos = tmpSlots.find(node);
    if (pos == tmpSlots.end()) {
      pos = tmpSlots.insert({node, -currentOffset}).first;
#ifdef STACK_SLOTS
      ir_printf("New TMP slot for node %n %N: %d\n", node, node, pos->second);
#endif
      currentOffset += 8;
    }

    return pos->second;
  }

  int32_t getLocVarUsedSize() const { return currentOffset; }
};

class AsmMethodPass : public FunctionPass<AsmMethodPass, Asm::Instruction> {
  StackSlotManager ssm;

public:
  AsmMethodPass(ir_graph *graph, Asm::Function *func) : FunctionPass(graph), func(func) {
    edges_activate(this->graph);
    inc_irg_visited(this->graph);
    ir_node *block = get_irg_start_block(this->graph);

    std::queue<ir_node *> blockStack;
    blockStack.push(block);

    while (!blockStack.empty()) {
      ir_node *curBlock = blockStack.front();
      blockStack.pop();

      if (irn_visited(curBlock))
        continue;

      func->newBB(curBlock);
      //std::cout << "Enlist " << nodeStr(curBlock) << std::endl;

      set_irn_visited(curBlock, get_irg_visited(this->graph));

      foreach_block_succ(curBlock, edge) {
        ir_node *pred = get_edge_src_irn(edge);
        blockStack.push(pred);
      }
    }

    // Reorder the blocks a bit. The start block is always the first one in the
    // topologic walk, but the end block isn't always the last so just move it there.
    for (size_t i = 0; i < func->orderedBasicBlocks.size(); i ++) {
      auto bb = func->orderedBasicBlocks.at(i);
      if (get_irg_end_block (graph) == bb->getNode()) {
        func->orderedBasicBlocks.erase(func->orderedBasicBlocks.begin() + i);
        func->orderedBasicBlocks.push_back(bb);
        break;
      }
    }

  }

  void before();
  void after() {
    func->setARSize(ssm.getLocVarUsedSize());
    // Sanity check
    assert(func->orderedBasicBlocks[0]->getNode() == get_irg_start_block(graph));
    // We can't assert the same thing for the end block since we have to allow
    // dead code.
  }

  void defaultVisitOp(ir_node *n) {
    ir_printf("  visiting node %n (%N)\n", n, n);
  }

  void writeValue(Asm::OperandPtr reg, ir_node *node, const std::string comment = "") {
    auto bb = getBB(node);

    bb->emplaceInstruction<Asm::Mov>(std::move(reg),
     std::make_unique<Asm::MemoryBase>(
          ssm.getStackSlot(node, bb),
          Asm::X86Reg(Asm::X86Reg::Name::bp,
                      Asm::X86Reg::Mode::R)),
     Asm::X86Reg::Mode::R,
                      //Asm::X86Reg::getRegMode(node))),
     std::move(comment));
  }

  Asm::BasicBlock* getBB(ir_node *n) {
    ir_node *block;
    if (is_Block(n))
      block = n;
    else
      block = get_nodes_block(n);

    return func->getBB(block);
  }

  void visitConv(ir_node *node);
  void visitCall(ir_node *node);
  void visitAdd(ir_node *node);
  void visitMul(ir_node *node);
  void visitCond(ir_node *node);
  void visitCmp(ir_node *node);
  void visitJmp(ir_node *node);
  void visitLoad(ir_node *node);
  void visitReturn(ir_node *node);
  void visitEnd(ir_node *node);
  void visitStore(ir_node *node);
  void visitSub(ir_node *node);
  void visitPhi(ir_node *node);
  void visitMod(ir_node *node);
  void visitDiv(ir_node *node);
  void visitMinus(ir_node *node);

  // Uninteresting nodes
  void visitProj(ir_node *node)    { PRINT_ORDER; }
  void visitBlock(ir_node *node)   { PRINT_ORDER; }
  void visitBad(ir_node *node)     { PRINT_ORDER; }
  void visitStart(ir_node *node)   { PRINT_ORDER; }
  void visitAddress(ir_node *node) { PRINT_ORDER; }
  void visitConst(ir_node *node)   { PRINT_ORDER; }

  Asm::OperandPtr getNodeResAsInstOperand(ir_node *node) {
    if (is_Const(node))
      return std::make_unique<Asm::Immediate>(get_Const_tarval(node));
    else if (is_Conv(node) && is_Const(get_Conv_op(node))) {
      return std::make_unique<Asm::Immediate>(get_Const_tarval(get_Conv_op(node)));
    }

    /* This assertion does not work for control flow/phi nodes, where we read first
     * and later generate the write instructions */
    //if (!ssm.hasSlot(node)) {
      //ir_printf("%n %N has no stack slot!\n", node, node);
      //assert(false);
    //}

    return std::make_unique<Asm::MemoryBase>(
        ssm.getStackSlot(node, getBB(node)),
        Asm::X86Reg(Asm::X86Reg::Name::bp,
                    Asm::X86Reg::Mode::R));
  }

  Asm::InstrPtr loadToReg(Asm::OperandPtr val, Asm::X86Reg reg) {
    return std::make_unique<Asm::Mov>(std::move(val), Asm::Register::get(reg));
  }

  Asm::InstrPtr writeResToStackSlot(Asm::X86Reg reg, ir_node *node) {
    return std::make_unique<Asm::Mov>(
        Asm::Register::get(reg, Asm::X86Reg::Mode::R),
        std::make_unique<Asm::MemoryBase>(
            ssm.getStackSlot(node, getBB(node)),
            Asm::X86Reg(Asm::X86Reg::Name::bp,
                        Asm::X86Reg::Mode::R)),
        Asm::X86Reg::Mode::R,
        "stack slot of " + nodeStr(node));
  }

private:
  Asm::Function *func;

  void generateNormalPhi(ir_node *node);
  void generateBoolPhi(ir_node *node);
  void generateSwapPhi(ir_node *node);
};

#endif
