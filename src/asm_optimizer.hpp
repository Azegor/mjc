#include "asm.hpp"

class AsmBlockOptimizer {
  Asm::Program *program;
public:
  AsmBlockOptimizer(Asm::Program *program) : program(program) {}

  void run();

  virtual void optimizeBlock(Asm::BasicBlock *block){ (void) block;}
};

class AsmFunctionOptimizer {
  Asm::Program *program;
public:
  AsmFunctionOptimizer(Asm::Program *program) : program(program) {}

  void run();

  virtual void optimizeFunction(Asm::Function *func){ (void) func;}
};


// ====================================================================
class AsmJumpOptimizer : public AsmFunctionOptimizer {
public:
  AsmJumpOptimizer(Asm::Program *program) : AsmFunctionOptimizer(program) {}
  void optimizeFunction(Asm::Function *func) override;
};

// ====================================================================
// Exchanges single slower instructions for single faster ones
class AsmSimpleOptimizer : public AsmBlockOptimizer {
public:
  AsmSimpleOptimizer(Asm::Program *program) : AsmBlockOptimizer(program) {}
  void optimizeBlock(Asm::BasicBlock *block) override;
};
