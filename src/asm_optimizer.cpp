#include <iostream>
#include <stdio.h>

#include "asm_optimizer.hpp"

void AsmBlockOptimizer::run() {
  for (auto &f : program->functions) {
    for (auto &b : f.basicBlocks) {
      if (b.second->instrs.size() > 0)
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
    if (bb->jumpInstrs.size() > 0) {
      auto lastJmp = &bb->jumpInstrs.back();
      if (*(lastJmp->ops[0].p.str.str) == nextBB->getLabelStr()) {
        // Just remove jump
        bb->jumpInstrs.pop_back();
        this->optimizations ++;
        continue;
      } else if (bb->jumpInstrs.size() >= 2) {
        // A basic block should contain either one jmp instruction or 2 jne/je/etc.
        // instructions. In the latter case, we can remove one of them if they just
        // jump to the next block anyway.
        auto secondToLastJmp = &bb->jumpInstrs.at(bb->jumpInstrs.size() - 2);
        if (secondToLastJmp->isJmp() &&
            *(secondToLastJmp->ops[0].p.str.str) == nextBB->getLabelStr()) {
          bb->jumpInstrs.erase(bb->jumpInstrs.end() - 2);
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
  for (size_t i = 0; i < block->instrs.size(); i ++) {
    auto instr = &block->instrs.at(i);
    if (instr->mnemonic == &Asm::Add &&
        instr->ops[0].type == Asm::OP_IMM) {
      if (instr->ops[0].p.imm.value == 0) {
        // Remove entirely
        block->removeInstr(i);
        this->optimizations ++;
      } else if (instr->ops[0].p.imm.value == 1) {
         // Replace with inc
         block->replaceInstr(i, &Asm::Inc, instr->ops[1]);
        this->optimizations ++;
      }
    } else if (instr->mnemonic == &Asm::Sub &&
                instr->ops[0].type == Asm::OP_IMM &&
                instr->ops[0].p.imm.value == 1) {
      // sub $1, reg
      // replace with dec
      block->replaceInstr(i, &Asm::Dec, instr->ops[1]);
      this->optimizations ++;
    } else if (instr->isMov() &&
                instr->ops[0].type == Asm::OP_IMM &&
                instr->ops[0].p.imm.value == 0 &&
                instr->ops[1].type != Asm::OP_IND) { // only handle mov $0, reg
      block->replaceInstr(i, &Asm::Xor, instr->ops[1], instr->ops[1]);
      this->optimizations ++;
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
  for (size_t i = 0; i < block->instrs.size() - 1; i ++) {
    auto mov1 = &block->instrs.at(i);
    auto mov2 = &block->instrs.at(i + 1);

    if (mov1->isMov() && mov2->isMov()) {
      if (mov1->ops[0].type == Asm::OP_REG &&
          mov1->ops[1].type == Asm::OP_IND &&
          mov2->ops[0].type == Asm::OP_IND &&
          mov2->ops[1].type == Asm::OP_REG) {
        // mov reg1, slot1
        // mov slot2, reg2
        if (mov1->ops[1].p.ind.base == mov2->ops[0].p.ind.base &&
            mov1->ops[1].p.ind.offset == mov2->ops[0].p.ind.offset) {
          // the 2 slots are the same

          if (mov1->ops[0].p.reg.name == mov2->ops[1].p.reg.name) {
            // mov reg, slot
            // mov slot, reg
            // -> Just remove the second mov!
            block->removeInstr(i + 1);
            this->optimizations ++;

            continue;
          }

          // the 2 registers are *not* the same.
          // mov reg1, slot
          // mov slot, reg2
          // -> replace second mov with mov from first reg to second reg
          block->replaceInstr(i + 1, Asm::makeMov(mov2->ops[1].p.reg.mode,
                                                  Asm::Op(mov1->ops[0].p.reg.name,
                                                          mov2->ops[1].p.reg.mode),
                                                  mov2->ops[1]));
          this->optimizations ++;
        }
      }
    }
  }
}

void AsmMovOptimizer::printOptimizations() {
  std::cout << "Removed " << this->optimizations << " redundant movs" << std::endl;
}



// ====================================================================
void AsmAliasOptimizer::optimizeBlock(Asm::BasicBlock *block) {
  (void)block;
#if 0
  for (size_t i = 0; i < block->instructions.size(); i ++) {
    auto mov = dynamic_cast<Asm::Mov*>(block->instructions.at(i).get());
    if (mov) {
      auto reg1 = dynamic_cast<Asm::Register*>(mov->src.get());
      auto reg2 = dynamic_cast<Asm::Register*>(mov->dest.get());

      if (reg1 && reg2) {
        for (size_t x = i; x < block->instructions.size(); x ++) {

        }

      }

    }
  }
#endif
}


void AsmAliasOptimizer::printOptimizations() {
  std::cout << "Removed " << this->optimizations << " redundant movs" << std::endl;
}
