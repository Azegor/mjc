#include "asm.hpp"

class AsmBlockOptimizer {
  Asm::Program *program;
public:
  AsmBlockOptimizer(Asm::Program *program) : program(program) {}

  void run();

  virtual void optimizeBlock(Asm::BasicBlock *block){ (void) block;}
};

// ====================================================================
// Exchanges single slower instructions for single faster ones
class AsmSimpleOptimizer : public AsmBlockOptimizer {
public:
  AsmSimpleOptimizer(Asm::Program *program) : AsmBlockOptimizer(program) {}
  void optimizeBlock(Asm::BasicBlock *block) override;
};
