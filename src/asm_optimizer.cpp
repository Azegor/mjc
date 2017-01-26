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
      if (*(lastJmp->ops[0].str.str) == nextBB->getLabelStr()) {
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
            *(secondToLastJmp->ops[0].str.str) == nextBB->getLabelStr()) {
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
// Removes mov instructions into stack slots we never read from again
void AsmStackOptimizer::optimizeFunction(Asm::Function *func) {
  const int slots = func->getARSize();
  bool *usedSlots = new bool[slots]();

  // First, find all stack slots we actually read out of
  for (size_t i = 0; i < func->orderedBasicBlocks.size(); i ++) {
    auto block = func->orderedBasicBlocks.at(i);
    for (size_t x = 0; x < block->flattenedInstrs.size(); x ++) {
      auto instr = &block->flattenedInstrs.at(x);

      if (instr->nOps >= 1 &&
          instr->ops[0].type == Asm::OP_IND) {
        // Mov into stack slot
        int offset = instr->ops[0].ind.offset;
        // Positive stack slots are function arguments
        if (offset < 0) {
          usedSlots[ (-offset) / 8] = true;
        }
      }

    }
  }

  // same loop, but this time check if the first operand of a mov
  // instruction is used.
  for (size_t i = 0; i < func->orderedBasicBlocks.size(); i ++) {
    auto block = func->orderedBasicBlocks.at(i);
    for (size_t x = 0; x < block->flattenedInstrs.size(); x ++) {
      auto instr = &block->flattenedInstrs.at(x);

      if (instr->isMov() &&
          instr->ops[1].type == Asm::OP_IND) {
        // Write into a stack slot.
        int offset = instr->ops[1].ind.offset;
        if (offset < 0 && !usedSlots[(- offset) / 8]) {
          // Write into a stack slot we never read out of again!
          // -> remove the instruction.
          block->removeFlattenedInstr(x);
          x --;
          this->optimizations ++;
        }
      }

    }
  }

  delete[] usedSlots;
}

void AsmStackOptimizer::printOptimizations() {
  std::cout << "Removed " << this->optimizations << " movs into unused stack slots" << std::endl;
}


// ====================================================================
void AsmSimpleOptimizer::optimizeBlock(Asm::BasicBlock *block) {
  for (size_t i = 0; i < block->instrs.size(); i ++) {
    auto instr = &block->instrs.at(i);
    if (instr->mnemonic == Asm::Add &&
        instr->ops[0].type == Asm::OP_IMM) {
      if (instr->ops[0].imm.value == 0) {
        // Remove entirely
        block->removeInstr(i);
        this->optimizations ++;
      } else if (instr->ops[0].imm.value == 1) {
         // Replace with inc
         block->replaceInstr(i, Asm::Inc, instr->ops[1]);
        this->optimizations ++;
      }
    } else if (instr->mnemonic == Asm::Sub &&
                instr->ops[0].type == Asm::OP_IMM &&
                instr->ops[0].imm.value == 1) {
      // sub $1, reg
      // replace with dec
      block->replaceInstr(i, Asm::Dec, instr->ops[1]);
      this->optimizations ++;
    } else if (instr->isMov() &&
                instr->ops[0].type == Asm::OP_IMM &&
                instr->ops[0].imm.value == 0 &&
                instr->ops[1].type != Asm::OP_IND) { // only handle mov $0, reg
      block->replaceInstr(i, Asm::Xor, instr->ops[1], instr->ops[1]);
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
  for (size_t i = 0; i < block->flattenedInstrs.size() - 1; i ++) {
    auto mov1 = &block->flattenedInstrs.at(i);
    auto mov2 = &block->flattenedInstrs.at(i + 1);

    if (mov1->isMov() && mov2->isMov()) {

      if (mov1->ops[0].type == Asm::OP_REG &&
          mov1->ops[1].type == Asm::OP_IND &&
          mov2->ops[0].type == Asm::OP_IND &&
          mov2->ops[1].type == Asm::OP_REG) {
        // mov reg1, slot1
        // mov slot2, reg2
        if (mov1->ops[1].ind.base == mov2->ops[0].ind.base &&
            mov1->ops[1].ind.offset == mov2->ops[0].ind.offset) {
          // the 2 slots are the same

          if (mov1->ops[0].reg.name == mov2->ops[1].reg.name) {
            // mov reg, slot
            // mov slot, reg
            // -> Just remove the second mov!
            block->removeFlattenedInstr(i + 1);
            this->optimizations ++;

            continue;
          }

          // the 2 registers are *not* the same.
          // mov reg1, slot
          // mov slot, reg2
          // -> replace second mov with mov from first reg to second reg
          block->replaceFlattenedInstr(i + 1, Asm::makeMov(mov2->ops[1].reg.mode,
                                                           Asm::Op(mov1->ops[0].reg.name,
                                                                   mov2->ops[1].reg.mode),
                                                           mov2->ops[1]));
          this->optimizations ++;
        }
      }
    }
  }

  // Walk instruction list backwards and remove unnecessary movs like
  // mov slot, reg
  // (instruction not touching reg)
  // miov slot, reg
  for (size_t i = block->flattenedInstrs.size() - 1; i > 0; i --) {
    auto instr1 = &block->flattenedInstrs.at(i);
    auto instr2 = &block->flattenedInstrs.at(i - 1); // !!!

    if (instr1->isMov() &&
        instr1->ops[0].type == Asm::OP_IND &&
        instr1->ops[1].type == Asm::OP_REG) {
      // instr1: mov slot, reg

      if (instr2->nOps >= 1) {
          // instr reg[, op2]
        if (instr2->ops[0].type == Asm::OP_REG &&
            instr2->ops[0].reg.name == instr1->ops[1].reg.name) {
          if (instr2->mnemonic == Asm::Inc ||
              instr2->mnemonic == Asm::Dec) {
            // modify their first op
            continue;
          }
        }

        if (instr2->nOps == 2) {
          // If an instruction has 2 operands, the second one is always modified unless...
          if (instr2->mnemonic != Asm::Cmp &&
              instr2->ops[1].type == Asm::OP_REG &&
              (instr2->ops[1].reg.name == instr1->ops[1].reg.name ||
               instr2->ops[1].reg.name == instr1->ops[0].ind.base)) {
            continue;
          }

          // Here we know insrt2 does not modify our register, so check the next one...
          if (i <= 1) continue;

          auto instr3 = &block->flattenedInstrs.at(i - 2);
          if (instr3->isMov() &&
              instr3->ops[0].type == Asm::OP_IND &&
              instr3->ops[0].ind.base == instr1->ops[0].ind.base &&
              instr3->ops[0].ind.offset == instr1->ops[0].ind.offset &&
              instr3->ops[1].type == Asm::OP_REG &&
              instr3->ops[1].reg.name == instr1->ops[1].reg.name) {
            // Ok, remove inst1.
            block->removeFlattenedInstr(i);
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



// ====================================================================
void AsmArrayOptimizer::optimizeBlock(Asm::BasicBlock *block) {
  for (size_t i = 0; i < block->flattenedInstrs.size() - 1; i ++) {
    auto mov1 = &block->flattenedInstrs.at(i);
    auto mov2 = &block->flattenedInstrs.at(i + 1);

    if (mov1->mnemonic == Asm::Add && mov2->isMov()) {
      // We are looking for
      // add $val, reg
      // mov op, (reg)
      // which is just longer for
      // mov op, val(reg)
      // So replace it with that.
      if (mov1->ops[0].type == Asm::OP_IMM &&
          mov1->ops[1].type == Asm::OP_REG &&
          mov2->ops[0].type != Asm::OP_IND &&
          mov2->ops[1].type == Asm::OP_IND &&
          mov2->ops[1].ind.base == mov1->ops[1].reg.name &&
          mov2->ops[1].ind.offset == 0) {

        int val = mov1->ops[0].imm.value;
        auto regName = mov1->ops[1].reg.name;
        auto regMode = mov1->ops[1].reg.mode;
        // Above case!
        // Remove add
        // Now mov2 is at pos i!
        mov2->ops[1] = Asm::Op(regName, regMode, val);
        block->removeFlattenedInstr(i);
        this->optimizations ++;
      }
    }
  }
}

void AsmArrayOptimizer::printOptimizations() {
  std::cout << "Replaced " << this->optimizations << " mov/add with just movs" << std::endl;
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
