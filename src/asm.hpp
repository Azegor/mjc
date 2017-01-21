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
#include <cstring>

#include <libfirm/firm.h>

namespace Asm {
using namespace std::string_literals;

// enum class OperandType : uint8_t {
//   Reg,       // value from register
//   Mem,       // value from constant memory adress
//   Indirect,  // value from addres in register
//   Const,     // (immediate) constant value
//   LocLabel,  // label of basic block
//   GlobLabel, // name of function
// };

enum class Opcode : uint32_t {
  Add,
  Sub,
  IMul,
  Div,
  Call,
  Cmp,
  Neg,
  Je ,
  Jne,
  Jmp,
  Jg ,
  Jge,
  Jl ,
  Jle,
  Inc,
  Dec,
  Xor,
  Mov,
  Movq,
  Movl,
  Movb,
  Movslq,
  Cqto,
  Label,
};

struct Mnemonic {
  Opcode opcode;
  const char *const name;
};
const Mnemonic Add    = { Opcode::Add, "add" };
const Mnemonic Sub    = { Opcode::Sub, "sub" };
const Mnemonic IMul   = { Opcode::IMul, "imul" };
const Mnemonic Div    = { Opcode::Div, "idivq" };
const Mnemonic Call   = { Opcode::Call, "call" };
const Mnemonic Cmp    = { Opcode::Cmp, "cmp" };
const Mnemonic Neg    = { Opcode::Neg, "neg" };
const Mnemonic Je     = { Opcode::Je, "je" };
const Mnemonic Jne    = { Opcode::Jne, "jne" };
const Mnemonic Jmp    = { Opcode::Jmp, "jmp" };
const Mnemonic Jg     = { Opcode::Jg, "jg" };
const Mnemonic Jge    = { Opcode::Jge, "jge" };
const Mnemonic Jl     = { Opcode::Jl, "jl" };
const Mnemonic Jle    = { Opcode::Jle, "jle" };
const Mnemonic Inc    = { Opcode::Inc, "inc" };
const Mnemonic Dec    = { Opcode::Dec, "dec" };
const Mnemonic Xor    = { Opcode::Xor, "xor" };
const Mnemonic Mov    = { Opcode::Mov, "mov" };
const Mnemonic Movq   = { Opcode::Movq, "movq" };
const Mnemonic Movl   = { Opcode::Movl, "movl" };
const Mnemonic Movb   = { Opcode::Movb, "movb" };
const Mnemonic Movslq = { Opcode::Movslq, "movslq" };
const Mnemonic Cqto   = { Opcode::Cqto, "cqto" };

const Mnemonic Label  = { Opcode::Label, "______" };

enum class RegName : uint8_t {
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

enum class RegMode : uint8_t {
  R, // 64 bit
  E, // 32 bit
  L, // 8 bit lower part
};

RegMode getRegMode(ir_node *node);

const char *getRegAsmName(const RegName name, const RegMode mode);

enum OpType {
  OP_IMM,
  OP_REG,
  OP_IND,
  OP_STR
};

struct Op {
  OpType type;
  union {
    struct {
      int value;
    } imm;
    struct {
      std::string *str;
    } str;
    struct {
      RegName name;
      RegMode mode;
    } reg;
    struct {
      RegName base;
      RegMode mode;
      int32_t offset;
    } ind;
  } p;

  Op() { type = static_cast<OpType>(-1); }
  Op(int v) {
    type = OP_IMM;
    p.imm.value = v;
  }
  Op(RegName regName, RegMode regMode) {
    type = OP_REG;
    p.reg.name = regName;
    p.reg.mode = regMode;
  }
  Op(RegName base, RegMode mode, int offset) {
    type = OP_IND;
    p.ind.base = base;
    p.ind.mode = mode;
    p.ind.offset = offset;
  }
  Op(const Op src, int offset) {
    if (src.type == OP_IND) {
      type = OP_IND;
      p.ind.base = src.p.ind.base;
      p.ind.mode = src.p.ind.mode;
      p.ind.offset = offset;
    } else if (src.type == OP_REG) {
      type = OP_IND;
      p.ind.base = src.p.reg.name;
      p.ind.mode = src.p.reg.mode;
      p.ind.offset = offset;
    } else
      assert(false);
  }
  Op(const std::string &str) {
    type = OP_STR;
    p.str.str = new std::string(str);
  }

  //~Op() {
    //if (type == OP_STR)
      //delete p.str.str;
  //}
};

std::ostream &operator<<(std::ostream &o, const Op &op);


struct Instr {
  Op ops[2];
  int nOps = -1;
  const Mnemonic *mnemonic;
  std::string comment;

  Instr(const Mnemonic *mne) {
    mnemonic = mne;
    nOps = 0;
  }
  Instr(const Mnemonic *mne, Op op1, Op op2, std::string c = ""s) {
    mnemonic = mne;
    nOps = 2;
    ops[0] = std::move(op1);
    ops[1] = std::move(op2);
    comment = std::move(c);
  }

  Instr(const Mnemonic *mne, Op op1, std::string c = ""s) {
    mnemonic = mne;
    nOps = 1;
    ops[0] = std::move(op1);
    comment = std::move(c);
  }

  bool isJmp() const {
    return mnemonic->opcode == Opcode::Jmp ||
           mnemonic->opcode == Opcode::Je ||
           mnemonic->opcode == Opcode::Jne ||
           mnemonic->opcode == Opcode::Jg ||
           mnemonic->opcode == Opcode::Jge ||
           mnemonic->opcode == Opcode::Jl ||
           mnemonic->opcode == Opcode::Jle;
  }

  bool isMov() const {
    return mnemonic->opcode == Opcode::Mov ||
           mnemonic->opcode == Opcode::Movq ||
           mnemonic->opcode == Opcode::Movl ||
           mnemonic->opcode == Opcode::Movb;
  }
};
std::ostream &operator<<(std::ostream &o, const Instr &instr);

// Convenience factory functions to create instructions/operands
Instr makeJump(const std::string target, ir_relation relation);
Instr makeMov(const RegMode mode, Op source, Op dest, std::string comment = ""s);
Op    rax();
Op    rbx();
Op    rcx();
Op    rbp();
Op    rsp();

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

class AsmWriter {
  std::ostream &out;

public:
  AsmWriter(std::ostream &o) : out(o) {}
  void write() { writeTextSection(); }

  void writeTextSection();

  void writeText(const std::string &text) { out << text << '\n'; }
  void writeString(const std::string &str) {
    out << '\t' << str << '\n';
  }

  void writeLabel(const std::string l) {
    out << l << ":\n";
  }

  void writeComment(const std::string &comment) { out << "/* -- " << comment << " */\n"; }

  void writeInstr(const Instr &instr) {
    if (instr.mnemonic->opcode != Opcode::Label)
      out << '\t';

    out << instr << '\n';
  }
};

std::string getBlockLabel(ir_node *node);

class BasicBlock {
  std::string comment;
  ir_node *node;

  std::vector<Instr> startPhiInstrs;
  std::vector<Instr> phiInstrs;

public:
  std::vector<Instr> instrs;
  std::vector<Instr> jumpInstrs;

  BasicBlock(ir_node *node, std::string comment = ""s)
    : comment(std::move(comment)), node(node) {}
  BasicBlock(BasicBlock &&bb) = default;

  void pushInstr(const Instr instr) {
    instrs.push_back(std::move(instr));
  }

  template<typename... Args>
  void pushInstr(Args &&... args) {
    instrs.push_back(Asm::Instr(std::forward<Args>(args)...));
  }

  template<typename... Args>
  void pushJumpInstr(Args &&... args) {
    jumpInstrs.push_back(Asm::Instr(std::forward<Args>(args)...));
  }

  template<typename... Args>
  void pushPhiInstr(Args &&... args) {
    phiInstrs.push_back(Asm::Instr(std::forward<Args>(args)...));
  }

  template<typename... Args>
  void pushStartPhiInstr(Args &&... args) {
    startPhiInstrs.push_back(Asm::Instr(std::forward<Args>(args)...));
  }

  template<typename... Args>
  void replaceInstr(size_t index, Args &&... args) {
    instrs.at(index) = Instr(args...);
  }

  void replaceInstr(size_t index, Instr instr) {
    instrs.at(index) = std::move(instr);
  }

  void removeInstr(size_t index) {
    this->instrs.erase(this->instrs.begin() + index);
  }

  void write(AsmWriter &writer) const {
    writer.writeLabel(getBlockLabel(node));


    if (startPhiInstrs.size() > 0) {
      writer.writeComment("------- StartPhiInstructions --------");
      for (auto &instr : startPhiInstrs) {
        writer.writeInstr(instr);
      }
    }

    if (instrs.size() > 0) {
      writer.writeComment("------- Normal Instructions --------");
      for (auto &instr : instrs) {
        writer.writeInstr(instr);
      }
    }

    if (phiInstrs.size() > 0) {
      writer.writeComment("------- PhiInstructions --------");
      for (auto &instr : phiInstrs) {
        writer.writeInstr(instr);
      }
    }

    if (jumpInstrs.size() > 0) {
      writer.writeComment("------- JumpInstructions --------");
      for (auto &instr : jumpInstrs) {
        writer.writeInstr(instr);
      }
    }
  }

  const std::string &getComment() const { return comment; }
  ir_node *getNode() { return node; }
  std::string getLabelStr() { return getBlockLabel(this->node); }
};

class Function {
  std::string fnName;
  int ARsize = 0;
  int startBlockId = -1;

public:
  std::vector<BasicBlock *> orderedBasicBlocks;
  std::unordered_map<ir_node *, BasicBlock *> basicBlocks;

  Function(std::string name) : fnName(std::move(name)) {}
  Function(Function &&) = default;

  ~Function () {
    for(auto bb : orderedBasicBlocks)
      delete bb;
  }

  void newBB(ir_node *node, std::string comment = ""s) {
    auto bb = new BasicBlock(node, comment);
    basicBlocks.insert({node, bb});
    orderedBasicBlocks.push_back(bb);
  }

  void setStartBlockId(int id) { startBlockId = id; }

  BasicBlock *getBB(ir_node *node) {
    assert(is_Block(node));

    if (basicBlocks.find(node) != basicBlocks.end()) {
      return basicBlocks.at(node);
    }
    return nullptr;
  }

  std::string getEpilogLabel() const {
    return fnName + "_epilog";
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
