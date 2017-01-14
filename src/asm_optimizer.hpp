#include "asm.hpp"

class AsmOptimizer {
  Asm::Program *program;
public:
  AsmOptimizer(Asm::Program *program) : program(program) {}

  void run();

  virtual void optimizeBlock(Asm::BasicBlock &block){ (void) block;}
};

// Exchanges single slower instructions for single faster ones
class AsmSimpleOptimizer : public AsmOptimizer {
public:
  AsmSimpleOptimizer(Asm::Program *program) : AsmOptimizer(program) {}
  void optimizeBlock(Asm::BasicBlock &block) override;
};
