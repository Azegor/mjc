#include <iostream>

#include "asm_optimizer.hpp"

void AsmBlockOptimizer::run() {
  for (auto &f : program->functions) {
    for (auto &b : f.basicBlocks) {
      if (b.second->instructions.size() > 0)
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
        this->optimizations ++;
        continue;
      } else if (bb->jumpInstructions.size() >= 2) {
        // A basic block should contain either one jmp instruction or 2 jne/je/etc.
        // instructions. In the latter case, we can remove one of them if they just
        // jump to the next block anyway.
        auto secondToLast = &bb->jumpInstructions.at(bb->jumpInstructions.size() - 2);
        auto secondToLastJmp = dynamic_cast<Asm::Jmp*>(secondToLast->get());
        if (secondToLastJmp && secondToLastJmp->targetLabel == nextBB->getLabelStr()) {
          bb->jumpInstructions.erase(bb->jumpInstructions.end() - 2);
          this->optimizations ++;
          continue;
        }
      }
    }
  }
}

void AsmJumpOptimizer::printOptimizations() {
  std::cout << "Removed " << this->optimizations << " redundant jumps" << std::endl;
}

// ====================================================================
void AsmSimpleOptimizer::optimizeBlock(Asm::BasicBlock *block) {
  for (size_t i = 0; i < block->instructions.size(); i ++) {
    auto instr = block->instructions.at(i).get();

    if (auto add = dynamic_cast<Asm::Add*>(instr)) {
      if (auto c = dynamic_cast<Asm::Immediate*>(add->src.get())) {
        if (c->getValue() == 1) {
          block->replaceInstruction<Asm::Inc>(i, std::move(add->dest));
          this->optimizations ++;
          continue;
        }
      }
    } else if (auto sub = dynamic_cast<Asm::Sub*>(instr)) {
      if (auto c = dynamic_cast<Asm::Immediate*>(sub->src.get())) {
        if (c->getValue() == 1) {
          block->replaceInstruction<Asm::Dec>(i, std::move(sub->dest));
          this->optimizations ++;
          continue;
        }
      }
    } else if (auto mov = dynamic_cast<Asm::Mov*>(instr)) {
      // It's unclear to me whether this is actually faster, but this way we can check
      // that the code to replace instructions actualy works. Also, it's what firm does.
      if (auto c = dynamic_cast<Asm::Immediate*>(mov->src.get())) {
        if (!dynamic_cast<Asm::MemoryBase*>(mov->dest.get()) && c->getValue() == 0) {
          block->replaceInstruction<Asm::Xor>(i, std::move(mov->dest));
          this->optimizations ++;
          continue;
        }
      }
    }
  }
}

void AsmSimpleOptimizer::printOptimizations() {
  std::cout << "Exchanged " << this->optimizations << " instructions for cheaper ones" << std::endl;
}

// ====================================================================
// For now, optimize cases of
// mov reg, slot
// mov slot, reg
//
// into just
// mov reg, slot
void AsmMovOptimizer::optimizeBlock(Asm::BasicBlock *block) {
  for (size_t i = 0; i < block->instructions.size() - 1; i ++) {
    auto instr = block->instructions.at(i).get();

    if (auto mov1 = dynamic_cast<Asm::Mov*>(instr)) {
      auto mov1Src = dynamic_cast<Asm::Register*>(mov1->src.get());
      auto mov1Dest = dynamic_cast<Asm::MemoryBase*>(mov1->dest.get());
      if (mov1Src && mov1Dest) {
        auto nextInstr = block->instructions.at(i + 1).get();
        if (auto mov2 = dynamic_cast<Asm::Mov*>(nextInstr)) {
          auto mov2Src = dynamic_cast<Asm::MemoryBase*>(mov2->src.get());
          auto mov2Dest = dynamic_cast<Asm::Register*>(mov2->dest.get());
          if (mov2Src && mov2Dest &&
              mov1Src->reg.name == mov2Dest->reg.name && // TODO: Check dest->base as well?
              mov1Dest->offset == mov2Src->offset) {
            // Remove the second mov instruction
            block->instructions.erase(block->instructions.begin() + i + 1);
            this->optimizations ++;
            continue;
          }
        }
      }
    }
  }
}

void AsmMovOptimizer::printOptimizations() {
  std::cout << "Removed " << this->optimizations << " redundant movs" << std::endl;
}
