#ifndef ASM_H
#define ASM_H

#include <vector>
#include <cstdint>

/// WORK IN PROGRESS!!!!

enum class OpCode : uint8_t {
  NOP,
  ADD,
  SUB,
  // ...
};

enum class Reg : uint8_t {
  RBP,
  RSP,
  RAX,
  // ...
};

struct Mem {
  // TODO
};

struct Constant {

};

enum class OperandType : uint8_t {
  Reg,
  Mem,
  Indirect,
  Const,
};

struct Operand {
  enum OperandType opType;
  union {
    Reg reg;
    Mem mem;
    Constant konst;
  };
};

struct Op {
  OpCode opCode;
  std::vector<Operand> operands;
};

#endif // ASM_H
