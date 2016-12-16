#ifndef ASM_H
#define ASM_H

#include <memory>
#include <ostream>
#include <string>
#include <typeinfo>
#include <vector>
#include <sstream>

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

  X86_64Register(Name n, Access a) : name(n), acc(a) {}

  const char* getAsmName() const;

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
  virtual ~Operand() {}
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

  static std::unique_ptr<Register> get(X86_64Register::Name name, X86_64Register::Access access) {
    return std::make_unique<Register>(X86_64Register(name, access));
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
  LocalLabel(const LocalLabel &) = default;
  LocalLabel(LocalLabel &&) = default;

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
  virtual ~Instruction() {}

  virtual Operand *getDestOperand() const = 0;
  virtual void write(std::ostream &o) const = 0;
  virtual bool isValid() const = 0; // call within assert

  friend std::ostream &operator<<(std::ostream &o, const Instruction &i) {
    assert(i.isValid());
    i.write(o);
    if (i.comment.length()) {
      o << " /* " << i.comment << " */";
    }
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

struct Comment : public Instruction {
  Comment(std::string comment) : Instruction(std::move(comment)) {}
  void write(std::ostream &) const override { /* comment printing done by Instruction */ }
  bool isValid() const override { return true; }
};

using OperandPtr = std::unique_ptr<Operand>;

struct ArithInstr : public Instruction {
  const OperandPtr src, dest;
  ArithInstr(OperandPtr s, OperandPtr d, std::string c = ""s)
      : Instruction(std::move(c)), src(std::move(s)), dest(std::move(d)) {}
  Operand *getDestOperand() const override { return dest.get(); }
  bool isValid() const override {
    return src->isOneOf<Immediate, Register, Memory>() && dest->isOneOf<Register, Memory>();
  }

  void writeInstr(std::ostream &o, const std::string &mnemonic) const {
    o << mnemonic << ' ' << *src << ", " << *dest;
  }
};

struct Add : public ArithInstr {
  Add(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Add); }
};

struct Sub : public ArithInstr {
  Sub(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Sub); }
};

struct Mul : public ArithInstr {
  Mul(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Mul); }
};

struct Div : public ArithInstr {
  Div(OperandPtr s, OperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Div); }
};

// compound types:

class AsmWriter {
  std::ostream &out;
public:
  AsmWriter(std::ostream &o) : out(o) {}
  void write() {
    writeTextSection();
  }

  void writeTextSection();

  void writeText(const std::string& text) {
    out << text << '\n';
  }
  void writeInstruction(const Instruction& instr) {
    out << '\t' << instr << '\n';
  }
  void writeLabel(const LocalLabel& label, const std::string &comment = ""s) {
    out << label << ':';
    if (comment.length()) {
      out << " /* " << comment << " */";
    }
    out << '\n';
  }
  void writeLabel(const NamedLabel& label) {
    out << label << ":\n";
  }
  void writeComment(const std::string &comment) {
    out << "/* -- " << comment << " */\n";
  }
};

class BasicBlock {
  std::string comment;
  const LocalLabel label;
  std::vector<InstrPtr> instructions;

public:
  BasicBlock(std::string comment = ""s) : comment(std::move(comment)), label() {}
  BasicBlock(LocalLabel l) : label(std::move(l)) {}
  BasicBlock(BasicBlock &&bb) = default;

  void addInstruction(InstrPtr instr) {
    instructions.emplace_back(std::move(instr));
  }
  template <typename T, typename... Args>
  void emplaceInstruction(Args&&... args) {
    addInstruction(std::make_unique<T>(std::forward<Args>(args)...));
  }

  void write(AsmWriter &writer) const {
    writer.writeLabel(label, comment);
    for (auto &instr : instructions) {
      writer.writeInstruction(*instr);
    }
  }

  const std::string &getComment() const { return comment; }
};

class Function {
  NamedLabel fnName;
  std::vector<BasicBlock> basicBlocks;

public:
  Function(std::string name) : fnName(std::move(name)) {}
  Function(NamedLabel l) : fnName(std::move(l)) {}
  Function(Function &&) = default;

  void addBB(BasicBlock bb) {
    basicBlocks.emplace_back(std::move(bb));
  }
  BasicBlock *newBB(std::string comment = ""s) {
    basicBlocks.emplace_back(std::move(comment));
    return &basicBlocks.back();
  }

  void write(AsmWriter &writer) const;
};


struct Programm {
  std::vector<Function> functions;

public:
  void addFunction(Function f) {
    functions.emplace_back(std::move(f));
  }

  friend std::ostream &operator<<(std::ostream &o, const Programm &p) {
    AsmWriter writer(o);
    writer.writeTextSection();
    for (auto &fn : p.functions) {
      fn.write(writer);
    }
    return o;
  }
};

}

#endif // ASM_H
