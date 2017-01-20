#include <iostream>

#include "asm_optimizer.hpp"

void AsmBlockOptimizer::run() {
  for (auto &f : program->functions) {
    for (auto &b : f.basicBlocks) {
      this->optimizeBlock(b.second);
    }
  }
}

void AsmFunctionOptimizer::run() {
  for (auto &f : program->functions) {
    this->optimizeFunction(&f);
  }
}


// ====================================================================
// Simply removes redundant jumps
void AsmJumpOptimizer::optimizeFunction(Asm::Function *func) {
  for (size_t i = 0; i < func->orderedBasicBlocks.size() - 1; i ++) {
    auto bb = func->orderedBasicBlocks.at(i);
    auto nextBB = func->orderedBasicBlocks.at(i + 1);
    if (bb->jumpInstructions.size() > 0) {
      auto lastJmp = dynamic_cast<Asm::Jmp*>(bb->jumpInstructions.back().get());
      //std::cout << lastJmp->targetLabel <<  "/" << nextBB->getLabelStr() << std::endl;
      if (lastJmp->targetLabel == nextBB->getLabelStr()) {
        // Just remove jump
        bb->jumpInstructions.pop_back();
        //std::cout << "Removing jump to " << lastJmp->targetLabel << std::endl;
        continue;
      }
    }
  }
}

// ====================================================================
void AsmSimpleOptimizer::optimizeBlock(Asm::BasicBlock *block) {
  for (size_t i = 0; i < block->instructions.size(); i ++) {
    auto instr = block->instructions.at(i).get();

    if (auto add = dynamic_cast<Asm::Add*>(instr)) {
      if (auto c = dynamic_cast<Asm::Immediate*>(add->src.get())) {
        if (c->getValue() == 1) {
          block->replaceInstruction<Asm::Inc>(i, std::move(add->dest));
          continue;
        }
      }
    } else if (auto mov = dynamic_cast<Asm::Mov*>(instr)) {
      // It's unclear to me whether this is actually faster, but this way we can check
      // that the code to replace instructions actualy works. Also, it's what firm does.
      if (auto c = dynamic_cast<Asm::Immediate*>(mov->src.get())) {
        if (!dynamic_cast<Asm::MemoryBase*>(mov->dest.get()) && c->getValue() == 0) {
          block->replaceInstruction<Asm::Xor>(i, std::move(mov->dest));
          continue;
        }
      }
    }
  }
}
