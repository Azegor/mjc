#ifndef ASM_H
#define ASM_H

#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <unordered_map>

#include <cassert>
#include <cstdint>

#include <libfirm/firm.h>

namespace Asm {

using namespace std::string_literals;

// TODO: everything

struct X86Reg {
  enum class Name : uint8_t {
    none,
    ax,
    bx,
    cx,
    dx,
    bp,
    sp,
    si,
    di,
    r8,
    r9,
    r10,
    r11,
    r12,
    r13,
    r14,
    r15,
  };
  enum class Mode : uint8_t {
    None,
    R, // 64 bit
    E, // 32 bit
    _, // 16 bit
    H, // 8 bit upper part
    L, // 8 bit lower part
  };

  const Name name;
  const Mode mode;

  X86Reg(Name n, Mode m) : name(n), mode(m) {}
  const char *getAsmName() const;

  static Mode getRegMode(ir_node *node);

  static X86Reg noReg;

private:
  X86Reg() : name(Name::none), mode(Mode::None) {}
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

  template <typename T> bool isOneOf() const { return dynamic_cast<const T *>(this) != nullptr; }
  template <typename T1, typename T2, typename... Args> bool isOneOf() const {
    return isOneOf<T1>() || isOneOf<T2, Args...>();
  }

  friend std::ostream &operator<<(std::ostream &o, const Operand &op) {
    op.write(o);
    return o;
  }
};

struct WritableOperand : public Operand {};

struct Register : public WritableOperand {
  X86Reg reg;
  Register(X86Reg r) : reg(r) {}

  void write(std::ostream &o) const override { o << reg.getAsmName(); }

  static std::unique_ptr<Register> get(X86Reg::Name name, X86Reg::Mode mode) {
    return get(X86Reg(name, mode));
  }
  static std::unique_ptr<Register> get(X86Reg reg) {
    return std::make_unique<Register>(reg);
  }
};

struct MemoryOperand : public WritableOperand {};
/// Memory is adressed the following way:
/// segment-override:signed-offset(base,index,scale)
/// variations: (be carefull wich are missing!)
/// (base)
/// (index, scale)
/// (base,index,scale)
/// signed-offset(base)
/// signed-offset(base,index)
/// signed-offset(index, scale)

/// offset and scale can always be specified (0 and 1 respectively) -> no extra classes
/// have Base, BaseIndex, IndexScale, BaseIndexScale

// Indirect specifies a memory location
struct MemoryBase : public MemoryOperand {

  // ignore segment-override
  int32_t offset;
  X86Reg base;
  MemoryBase(int32_t offset, X86Reg base) : offset(offset), base(base) {}

  void write(std::ostream &o) const override {
    // TODO: incorporate all values
    if (offset) {
      o << offset;
    }
    o << '(' << base.getAsmName() << ')';
  }
};

struct MemoryIndex : public MemoryOperand {

  // ignore segment-override
  int32_t offset;
  X86Reg base;
  X86Reg index;
  int32_t scale;
  MemoryIndex(int32_t offset, X86Reg base, X86Reg index = X86Reg::noReg,
              int32_t scale = 1)
      : offset(offset), base(base), index(index), scale(scale) {
    assert(scale); // may not be missing / zero
  }

  void write(std::ostream &o) const override {
    // TODO: incorporate all values
    if (offset) {
      o << offset;
    }
    o << '(' << index.getAsmName() << ',' << scale << ')';
  }
};

struct MemoryBaseIndex : public MemoryOperand {

  // ignore segment-override
  int32_t offset;
  X86Reg base;
  X86Reg index;
  int32_t scale;
  MemoryBaseIndex(int32_t offset, X86Reg base, X86Reg index = X86Reg::noReg,
                  int32_t scale = 1)
      : offset(offset), base(base), index(index), scale(scale) {}

  void write(std::ostream &o) const override {
    // TODO: incorporate all values
    if (offset) {
      o << offset;
    }
    o << '(' << base.getAsmName() << ',' << index.getAsmName() << ',' << scale << ')';
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
  LocalLabel(uint32_t nr) : nr(nr) {}

  void write(std::ostream &o) const override { o << ".L" << nr; }

  static uint32_t newNr() {
    static uint32_t curVal = 0;
    return ++curVal;
  }
};

struct NamedLabel : public Operand {
  std::string name;
  NamedLabel(std::string n) : name(std::move(n)) {}

  void write(std::ostream &o) const override { o << name; }
};

/// --- instructions ---

struct Instruction {
  const std::string comment;

  Instruction(std::string comment) : comment(std::move(comment)) {}
  virtual ~Instruction() {}

  virtual Operand *getDestOperand() const = 0;
  virtual void write(std::ostream &o) const = 0;
  virtual bool isValid() const = 0; // call within assert

  void writeInstr(std::ostream &o, const std::string &mnemonic, const Operand *o1,
                  const Operand *o2) const {
    o << mnemonic << ' ' << *o1 << ", " << *o2;
  }

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
const char *const Nop  = "nop";
const char *const Add  = "add";
const char *const Sub  = "sub";
const char *const IMul = "imul";
const char *const Div  = "div";
const char *const Call = "call";
const char *const Cmp  = "cmp";
const char *const Je   = "je";
const char *const Jne  = "jne";
const char *const Jmp  = "jmp";
const char *const Jg   = "jg";
const char *const Jge  = "jge";
const char *const Jl   = "jl";
const char *const Jle  = "jle";

const char *const Mov  = "mov";
const char *const Movq = "movq";
const char *const Movl = "movl";

// GAS inferrs the operand type if not specified (b, s, w, l, q, t)
}

using OperandPtr = std::unique_ptr<Operand>;
using WritableOperandPtr = std::unique_ptr<WritableOperand>;

struct Call : public Instruction {
  std::string functionName;
  Call(std::string functionName) : Instruction("call"), functionName(functionName) {}
  void write(std::ostream &o) const override {
    o << mnemonic::Call << " " << functionName;
  }
  Operand *getDestOperand() const override {
    assert(0);
    return nullptr;
  }
  bool isValid() const override { return true; }
};

struct Cmp : public Instruction {
  const OperandPtr left;
  const OperandPtr right;

  Cmp(OperandPtr left, OperandPtr right) :
    Instruction("cmp"), left(std::move(left)), right(std::move(right)) {}
  bool isValid() const override { return true; }
  Operand *getDestOperand() const override {
    assert(false);
    return nullptr;
  }
  void write(std::ostream &o) const override {
    o << mnemonic::Cmp << ' ' << *left << ", " << *right;
  }
};

struct Jmp : public Instruction {
  ir_relation relation;
  std::string targetLabel;
  Jmp(const std::string targetLabel, ir_relation relation)
    : Instruction(""), relation(relation), targetLabel(std::move(targetLabel)) {}

  Operand *getDestOperand() const override {
    assert(false);
    return nullptr;
  }
  void write(std::ostream &o) const override {
    switch(relation) {
      case ir_relation_equal:
        o << mnemonic::Je;
        break;
      case ir_relation_less_greater:
        o << mnemonic::Jne;
        break;
      case ir_relation_true:
        o << mnemonic::Jmp;
        break;
      case ir_relation_greater:
        o << mnemonic::Jg;
        break;
      case ir_relation_greater_equal:
        o << mnemonic::Jge;
        break;
      case ir_relation_less:
        o << mnemonic::Jl;
        break;
      case ir_relation_less_equal:
        o << mnemonic::Jle;
        break;
      default:
        assert(false);
    }
    o << " ." << targetLabel;
  }
  bool isValid() const override { return true; }
};

struct Nop : public Instruction {
  Nop(std::string comment = ""s) : Instruction(std::move(comment)) {}
  Operand *getDestOperand() const override {
    assert(false);
    return nullptr;
  }
  void write(std::ostream &o) const override { o << mnemonic::Nop; }
  bool isValid() const override { return true; }
};

struct Comment : public Instruction {
  Comment(std::string comment) : Instruction(std::move(comment)) {}
  void write(std::ostream &) const override { /* comment printing done by Instruction */
  }
  Operand *getDestOperand() const override {
    assert(false);
    return nullptr;
  }
  bool isValid() const override { return true; }
};

struct ArithInstr : public Instruction {
  const OperandPtr src;
  const WritableOperandPtr dest;
  ArithInstr(OperandPtr s, WritableOperandPtr d, std::string c = ""s)
      : Instruction(std::move(c)), src(std::move(s)), dest(std::move(d)) {}
  Operand *getDestOperand() const override { return dest.get(); }
  bool isValid() const override {
    // TODO: is this still necessary? we can enforce this via the typesystem
    return src->isOneOf<Immediate, WritableOperand>() && dest->isOneOf<WritableOperand>();
  }

  void writeInstr(std::ostream &o, const std::string &mnemonic) const {
    o << mnemonic << ' ' << *src << ", " << *dest;
  }
};

struct Add : public ArithInstr {
  Add(OperandPtr s, WritableOperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Add); }
};

struct Sub : public ArithInstr {
  Sub(OperandPtr s, WritableOperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Sub); }
};

struct Mul : public ArithInstr {
  Mul(OperandPtr s, WritableOperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::IMul); }
};

struct Div : public ArithInstr {
  Div(OperandPtr s, WritableOperandPtr d, std::string c = ""s)
      : ArithInstr(std::move(s), std::move(d), std::move(c)) {}

  void write(std::ostream &o) const override { writeInstr(o, mnemonic::Div); }
};

//

struct Mov : public Instruction {
  const OperandPtr src;
  const OperandPtr dest;

  Mov(OperandPtr s, OperandPtr d, std::string c = ""s)
      : Instruction(std::move(c)), src(std::move(s)), dest(std::move(d)) {}

  Operand *getDestOperand() const override { return dest.get(); }
  bool isValid() const override {
    return true; // TODO
    //     return src->isOneOf<Immediate, WritableOperand>() && dest->isOneOf<WritableOperand>();
  }

  void write(std::ostream &o) const override {
    // XXX Ehm this is a little ugly
    if (auto d = dynamic_cast<Asm::Register*>(dest.get())) {
      if (d->reg.mode == X86Reg::Mode::L)
        writeInstr(o, mnemonic::Movl, src.get(), dest.get());
      else
        writeInstr(o, mnemonic::Movq, src.get(), dest.get());
    }else if (auto d = dynamic_cast<Asm::MemoryBase*>(dest.get())) {
      if (d->base.mode == X86Reg::Mode::L)
        writeInstr(o, mnemonic::Movl, src.get(), dest.get());
      else
        writeInstr(o, mnemonic::Movq, src.get(), dest.get());
    }else {
      writeInstr(o, mnemonic::Mov, src.get(), dest.get());
    }
  }
};

// compound types:

class AsmWriter {
  std::ostream &out;

public:
  AsmWriter(std::ostream &o) : out(o) {}
  void write() { writeTextSection(); }

  void writeTextSection();

  void writeText(const std::string &text) { out << text << '\n'; }
  void writeInstruction(const Instruction &instr) { out << '\t' << instr << '\n'; }
  void writeInstruction(const std::string &instr, const std::string &comment = ""s) {
    writeImpl('\t' + instr, comment);
  }
  void writeLabel(const LocalLabel &label, const std::string &comment = ""s) {
    writeLabelImpl(label, comment);
  }
  void writeLabel(const NamedLabel &label, const std::string &comment = ""s) {
    writeLabelImpl(label, comment);
  }

  void writeComment(const std::string &comment) { out << "/* -- " << comment << " */\n"; }


private:
  template <typename T>
  void writeLabelImpl(const T& label, const std::string &comment) {
    out << label << ':';
    if (comment.length()) {
      out << " /* " << comment << " */";
    }
    out << '\n';
  }
  template <typename T>
  void writeImpl(const T& content, const std::string &comment) {
    out << content;
    if (comment.length()) {
      out << " /* " << comment << " */";
    }
    out << '\n';
  }
};

class BasicBlock {
  std::string comment;
  const LocalLabel label;
  std::vector<InstrPtr> instructions;
  InstrPtr jumpInstruction;

public:
  BasicBlock(std::string comment = ""s) : comment(std::move(comment)), label() {}
  BasicBlock(int num, std::string comment = ""s) : comment(std::move(comment)), label(num) {}
  BasicBlock(LocalLabel l) : label(std::move(l)) {}
  BasicBlock(BasicBlock &&bb) = default;

  void addInstruction(InstrPtr instr) { instructions.emplace_back(std::move(instr)); }

  template <typename T, typename... Args>
  Instruction *emplaceInstruction(Args &&... args) {
    auto instr = std::make_unique<T>(std::forward<Args>(args)...);
    auto res = instr.get(); // save before move
    addInstruction(std::move(instr));
    return res;
  }

  template <typename... Args>
  void emplaceJump(Args &&... args) {
    this->jumpInstruction = std::make_unique<Asm::Jmp>(std::forward<Args>(args)...);
  }

  void addComment(const std::string comment) {
    this->emplaceInstruction<Asm::Comment>(std::move(comment));
  }


  void write(AsmWriter &writer) const {
    writer.writeLabel(label, comment);
    for (auto &instr : instructions) {
      writer.writeInstruction(*instr);
    }

    if (jumpInstruction != nullptr)
      writer.writeInstruction(*jumpInstruction);

  }

  const std::string &getComment() const { return comment; }
};

class Function {
  NamedLabel fnName;
  std::unordered_map<ir_node *, BasicBlock> basicBlocks;
  int ARsize = 0;
  int startBlockId = -1;

public:
  Function(std::string name) : fnName(std::move(name)) {}
  Function(NamedLabel l) : fnName(std::move(l)) {}
  Function(Function &&) = default;

  BasicBlock *newBB(ir_node *node, std::string comment = ""s) {
    basicBlocks.emplace(std::piecewise_construct,
                        std::make_tuple(node),
                        std::make_tuple(get_irn_node_nr(node), std::move(comment)));
    return &basicBlocks[node];
  }

  void setStartBlockId(int id) { startBlockId = id; }

  BasicBlock *getBB(ir_node *node) {
    return &basicBlocks[node];
  }

  std::string getEpilogLabel() const {
    return fnName.name + "_epilog";
  }

  void setARSize(int size) { ARsize = size; }

  void writeProlog(AsmWriter &writer) const;
  void writeEpilog(AsmWriter &writer) const;

  void write(AsmWriter &writer) const;
};

struct Program {
  std::vector<Function> functions;

public:
  void addFunction(Function f) { functions.emplace_back(std::move(f)); }

  friend std::ostream &operator<<(std::ostream &o, const Program &p) {
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
