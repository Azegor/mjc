#ifndef ASM_H
#define ASM_H

#include <memory>
#include <ostream>
#include <string>
#include <typeinfo>
#include <vector>

#include <cstdint>
#include <cassert>

struct ir_tarval;
long get_tarval_long (ir_tarval const *tv);

namespace Asm {

using namespace std::string_literals;

// TODO: everything

struct X86_64Register {
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

  const char* getAsmName() const {
    switch (acc) {
      case Access::R: {
          switch(name) {
            case Name::rax: return "%rax";
            case Name::rbx: return "%rbx";
            case Name::rcx: return "%rcx";
            case Name::rdx: return "%rdx";
            case Name::rbp: return "%rbp";
            case Name::rsp: return "%rsp";
            case Name::rsi: return "%rsi";
            case Name::rdi: return "%rdi";
            case Name::r8 : return "%r8";
            case Name::r9 : return "%r9";
            case Name::r10: return "%r10";
            case Name::r11: return "%r11";
            case Name::r12: return "%r12";
            case Name::r13: return "%r13";
            case Name::r14: return "%r14";
            case Name::r15: return "%r15";
          }
      }
      case Access::E: {
          switch(name) {
            case Name::rax: return "%eax";
            case Name::rbx: return "%ebx";
            case Name::rcx: return "%ecx";
            case Name::rdx: return "%edx";
            case Name::rbp: return "%ebp";
            case Name::rsp: return "%esp";
            case Name::rsi: return "%esi";
            case Name::rdi: return "%edi";
            case Name::r8 : return "%e8";
            case Name::r9 : return "%e9";
            case Name::r10: return "%e10";
            case Name::r11: return "%e11";
            case Name::r12: return "%e12";
            case Name::r13: return "%e13";
            case Name::r14: return "%e14";
            case Name::r15: return "%e15";
          }
      }
      default:
        // TODO
        __builtin_trap();
    }
  }

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


/// --- instruction operands ---

struct Operand {
  virtual void write(std::ostream &o) const = 0;

  template <typename T> bool isOneOf() const { return typeid(T) == typeid(*this); }
  template <typename T1, typename T2, typename... Args> bool isOneOf() const {
    return isOneOf<T1>() || isOneOf<T2, Args...>();
  }

  friend std::ostream &operator<<(std::ostream &o, const Operand &op) {
    op.write(o);
    return o;
  }
};

struct Register : public Operand {
  X86_64Register reg;
  Register(X86_64Register r) : reg(r) {}

  void write(std::ostream &o) const override {
    o << reg.getAsmName();
  }
};

// Indirect specifies a memory location
struct Memory : public Operand {
  /// Memory is adressed the following way:
  /// segment-override:signed-offset(base,index,scale)

  // ignore segment-override
  ir_tarval *offset;
  X86_64Register base;
  X86_64Register index;
  ir_tarval *scale;
  Memory(ir_tarval *offset, X86_64Register base, X86_64Register index, ir_tarval *scale) :
    offset(offset), base(base), index(index), scale(scale) {}

  void write(std::ostream &o) const override {
    // TODO: incorporate all values
    o << '(' << base.getAsmName() << ')';
  }
};

struct Immediate : public Operand {
  ir_tarval *val;
  Immediate(ir_tarval *val) : val(val) {}

  void write(std::ostream &o) const override {
    o << "$" << get_tarval_long(val);
//     o << "$0x" << std::hex << get_tarval_long(val);
  }
};

struct LocalLabel : public Operand {
  uint32_t nr;
  LocalLabel() : nr(newNr()) {}

  void write(std::ostream &o) const override {
    o << ".L" << nr;
  }

  static uint32_t newNr() {
    static uint32_t curVal = 0;
    return ++curVal;
  }
};

struct NamedLabel : public Operand {
  std::string name;
  NamedLabel(std::string n) : name(std::move(n)) {}

  void write(std::ostream &o) const override {
    o << name;
  }
};

/// --- instructions ---

struct Instruction {
  const std::string comment;

  Instruction(std::string comment) : comment(std::move(comment)) {}

  virtual Operand *getDestOperand() const = 0;
  virtual void write(std::ostream &o) const = 0;
  virtual bool isValid() const = 0; // call within assert

  friend std::ostream &operator<<(std::ostream &o, const Instruction &i) {
    assert(i.isValid());
    o << '\t';
    i.write(o);
    o << i.comment << '\n';
    return o;
  }
};

// for assembly code in string form (predefined assembly code)
struct StringInstructionBlock : public Instruction {
  std::string content;
  StringInstructionBlock(std::string content, std::string comment)
      : Instruction(std::move(comment)), content(std::move(content)) {}

  virtual void write(std::ostream &o) const {
    o << "/* begin custom assembly code */\n";
    o << content << "\n\t/* end custom assembly code */";
    // here comes the comment from Instruction::friend operator<<
  }
};

using InstrPtr = std::unique_ptr<Instruction>;

/// --- x86 instructions ---

namespace mnemonic {
const char * const Nop = "nop";
const char * const Add = "add";
const char * const Sub = "sub";
const char * const Mul = "mul";
const char * const Div = "div";

// GAS inferrs the operand type if not specified (b, s, w, l, q, t)
}

struct Nop : public Instruction {
  Nop(std::string comment = ""s) : Instruction(std::move(comment)) {}
  Operand *getDestOperand() const override { assert(false); return nullptr; }
  void write(std::ostream &o) const override { o << mnemonic::Nop; }
  bool isValid() const override { return true; }
};

using OperandPtr = std::unique_ptr<Operand>;

struct ArithInstr : public Instruction {
  const OperandPtr src, dest;
  ArithInstr(OperandPtr s, OperandPtr d, std::string c = ""s)
      : Instruction(std::move(c)), src(std::move(s)), dest(std::move(d)) {}
  bool isValid() const override {
    return src->isOneOf<Immediate, Register, Memory>() && dest->isOneOf<Register, Memory>();
  }
};

struct Add : public ArithInstr {
  Add(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { o << mnemonic::Add << *src << ", " << *dest; }
};

struct Sub : public ArithInstr {
  Sub(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { o << mnemonic::Sub << *src << ", " << *dest; }
};

struct Mul : public ArithInstr {
  Mul(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { o << mnemonic::Mul << *src << ", " << *dest; }
};

struct Div : public ArithInstr {
  Div(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { o << mnemonic::Div << *src << ", " << *dest; }
};

}

#endif // ASM_H
