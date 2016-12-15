#ifndef ASM_H
#define ASM_H

#include <memory>
#include <string>
#include <ostream>
#include <vector>

#include <cstdint>

using namespace std::string_literals;

struct ir_tarval;

// TODO: everything

struct Register {
  enum class Name : uint8_t {
    rax,
    rbx,
    rcx,
    rdx,
    rbp,
    rsp,
    rsi,
    rdi,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
  };
  enum class Access : uint8_t {
    R, // 64 bit
    E, // 32 bit
    _, // 16 bit
    H, // 8 bit upper part
    L, // 8 bit lower part
  };

  const Name name;
  const Access acc;
};

// enum class OperandType : uint8_t {
//   Reg,       // value from register
//   Mem,       // value from constant memory adress
//   Indirect,  // value from addres in register
//   Const,     // (immediate) constant value
//   LocLabel,  // label of basic block
//   GlobLabel, // name of function
// };

struct Operand {
  virtual void write(std::ostream &o) = 0;
};

struct RegOperand : public Operand {
  Register reg;
  RegOperand(Register r) : reg(r) {}
};

struct IndirectOperand : public Operand {
  Register reg;
  IndirectOperand(Register r) : reg(r) {}
};

struct MemOperand : public Operand {
  // TODO
};

struct ConstOperand : public Operand {
  ir_tarval *val;
};

struct LocalLabelOperand : public Operand {
  uint32_t nr;
};

struct NamedLabelOperand : public Operand {
  std::string name;
};

struct Instruction {
  const std::string comment;
  Instruction(std::string comment) : comment(std::move(comment)) {}

  friend std::ostream &operator<<(std::ostream& o, const Instruction &i) {
    o << '\t';
    i.write(o);
    o << i.comment << '\n';
    return o;
  }
  virtual void write(std::ostream &o) const = 0;
  virtual bool validate() const = 0; // call within assert
};

// for assembly code in string form (predefined assembly code)
struct StringInstructionBlock : public Instruction {
  std::string content;
  StringInstructionBlock(std::string content, std::string comment) : Instruction(std::move(comment)), content(std::move(content)) {}

  virtual void write(std::ostream &o) const {
    o << "/* begin custom assembly code */\n";
    o << content << '\n';
    o << '\t'; // here comes the comment from Instruction::friend operator<<
  }
};

struct Nop : public Instruction {
  Nop(std::string comment = ""s) : Instruction(std::move(comment)) {}
  void write(std::ostream &o) const override {
    o << "nop";
  }
  bool validate() const override { return true; }
};

using OperandPtr = std::unique_ptr<Operand>;

struct Add : public Instruction {
  const OperandPtr left, right;
  Add(std::string comment, OperandPtr l, OperandPtr r)
      : Instruction(std::move(comment)), left(std::move(l)), right(std::move(r)) {}

  void write(std::ostream &o) const override {
    o << "add";
  }
  bool validate() const override { return true; }
};

#endif // ASM_H
