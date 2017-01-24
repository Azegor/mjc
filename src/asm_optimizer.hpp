#include "asm.hpp"

class AsmBlockOptimizer {
  Asm::Program *program;
protected:
  int optimizations = 0;
public:
  AsmBlockOptimizer(Asm::Program *program) : program(program) {}

  void run();

  virtual void optimizeBlock(Asm::BasicBlock *block){ (void) block;}
  virtual void printOptimizations() = 0;
};

class AsmFunctionOptimizer {
  Asm::Program *program;
protected:
  int optimizations = 0;
public:
  AsmFunctionOptimizer(Asm::Program *program) : program(program) {}

  void run();

  virtual void optimizeFunction(Asm::Function *func){ (void) func;}
  virtual void printOptimizations() = 0;
};


// ====================================================================
class AsmJumpOptimizer : public AsmFunctionOptimizer {
public:
  AsmJumpOptimizer(Asm::Program *program) : AsmFunctionOptimizer(program) {}
  void optimizeFunction(Asm::Function *func) override;
  void printOptimizations() override;
};

// ====================================================================
class AsmStackOptimizer : public AsmFunctionOptimizer {
public:
  AsmStackOptimizer(Asm::Program *program) : AsmFunctionOptimizer(program) {}
  void optimizeFunction(Asm::Function *func) override;
  void printOptimizations() override;
};

// ====================================================================
// Exchanges single slower instructions for single faster ones
class AsmSimpleOptimizer : public AsmBlockOptimizer {
public:
  AsmSimpleOptimizer(Asm::Program *program) : AsmBlockOptimizer(program) {}
  void optimizeBlock(Asm::BasicBlock *block) override;
  void printOptimizations() override;
};

// ====================================================================
// Removes redundant mov operations
class AsmMovOptimizer : public AsmBlockOptimizer {
public:
  AsmMovOptimizer(Asm::Program *program) : AsmBlockOptimizer(program) {}
  void optimizeBlock(Asm::BasicBlock *block) override;
  void printOptimizations() override;
};

// ====================================================================
// Finds mov reg1, reg2 instructions and replaces
// reg2 with reg1 in all following instructions until
// a mov foo, reg2
class AsmAliasOptimizer : public AsmBlockOptimizer {
  AsmAliasOptimizer(Asm::Program *program) : AsmBlockOptimizer(program) {}
  void optimizeBlock(Asm::BasicBlock *block) override;
  void printOptimizations() override;
};
