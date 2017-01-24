#ifndef ASM_H
#define ASM_H

#include <memory>
#include <ostream>
#include <iostream>
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

struct Mnemonic {
  uint32_t opcode;
  const char *const name;
};

extern const Mnemonic *Foo;
//= new Mnemonic{0, "foo"};

extern const Mnemonic *Add;
extern const Mnemonic *Sub;
extern const Mnemonic *IMul;
extern const Mnemonic *Div;
extern const Mnemonic *Call;
extern const Mnemonic *Cmp;
extern const Mnemonic *Neg;
extern const Mnemonic *Je;
extern const Mnemonic *Jne;
extern const Mnemonic *Jmp;
extern const Mnemonic *Jg;
extern const Mnemonic *Jge;
extern const Mnemonic *Jl;
extern const Mnemonic *Jle;
extern const Mnemonic *Inc;
extern const Mnemonic *Dec;
extern const Mnemonic *Xor;
extern const Mnemonic *Mov;
extern const Mnemonic *Movq;
extern const Mnemonic *Movl;
extern const Mnemonic *Movb;
extern const Mnemonic *Movslq;
extern const Mnemonic *Cqto;
extern const Mnemonic *Label;

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
  OP_STR,
  OP_NONE
};

struct Op {
private:
  struct _imm {
    int value;
  };
  struct _str {
    std::string *str = nullptr;
  };
  struct _reg {
    RegName name;
    RegMode mode;
  };
  struct _ind {
    RegName base;
    RegMode mode;
    int32_t offset;
  };
public:
  OpType type = OP_NONE;
  union {
    _imm imm;
    _str str;
    _reg reg;
    _ind ind;
  };

  Op() { type = OP_NONE; str.str = nullptr; }
  Op(int v) {
    type = OP_IMM;
    imm.value = v;
  }
  Op(RegName regName, RegMode regMode) {
    type = OP_REG;
    reg.name = regName;
    reg.mode = regMode;
  }
  Op(RegName base, RegMode mode, int offset) {
    type = OP_IND;
    ind.base = base;
    ind.mode = mode;
    ind.offset = offset;
  }
  Op(const Op &src, int offset) {
    if (src.type == OP_IND) {
      type = OP_IND;
      ind.base = src.ind.base;
      ind.mode = src.ind.mode;
      ind.offset = offset;
    } else if (src.type == OP_REG) {
      type = OP_IND;
      ind.base = src.reg.name;
      ind.mode = src.reg.mode;
      ind.offset = offset;
    } else
      assert(false);
  }
  Op(const std::string &s) {
    type = OP_STR;
    str.str = new std::string(s);
  }

  Op(const Op &op) {
    type = op.type;
    switch(op.type) {
      case OP_IMM:
        imm.value = op.imm.value;
        break;
      case OP_REG:
        reg = op.reg;
        break;
      case OP_IND:
        ind = op.ind;
        break;
      case OP_STR:
        if (op.str.str != nullptr) {
          str.str = new std::string(*op.str.str);
        }
        break;
      case OP_NONE:
        break;
      default:
        assert(false);
    }
  }

  ~Op() {
    if (type == OP_STR && str.str != nullptr) {
      delete str.str;
    }
  }

  Op &operator=(const Op& other) {
    if (type == OP_STR && str.str != nullptr)
      delete str.str;

    type = other.type;
    switch(type) {
      case OP_IMM:
        imm.value = other.imm.value;
        break;
      case OP_REG:
        reg = other.reg;
        break;
      case OP_IND:
        ind = other.ind;
        break;
      case OP_STR:
        str.str = new std::string(*other.str.str);
        break;
      default:
        assert(false);
    }

    return *this;
  }

  Op &operator=(Op&& other) {
    if (type == OP_STR && str.str != nullptr)
      delete str.str;

    type = other.type;
    switch(type) {
      case OP_IMM:
        imm.value = other.imm.value;
        break;
      case OP_REG:
        reg = other.reg;
        break;
      case OP_IND:
        ind = other.ind;
        break;
      case OP_STR:
        str.str = new std::string(*other.str.str);
        break;
      default: {}
    }

    return *this;
  }
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
    return mnemonic == Jmp ||
           mnemonic == Je  ||
           mnemonic == Jne ||
           mnemonic == Jg  ||
           mnemonic == Jge ||
           mnemonic == Jl  ||
           mnemonic == Jle;
  }

  bool isMov() const {
    return mnemonic == Mov ||
           mnemonic == Movq ||
           mnemonic == Movl ||
           mnemonic == Movb;
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
    if (instr.mnemonic != Label)
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
  // Keep this separate for debugging
  std::vector<Instr> flattenedInstrs;


  BasicBlock(ir_node *node, std::string comment = ""s)
    : comment(std::move(comment)), node(node) {}
  BasicBlock(BasicBlock &&bb) = default;


  void flattenInstrs() {
    assert(flattenedInstrs.size() == 0);
    // So far, we have several lists of instructions, now
    // flatten them all to one list.

    for (auto &instr : startPhiInstrs) {
      flattenedInstrs.push_back(instr);
    }

    for (auto &instr : instrs) {
      flattenedInstrs.push_back(instr);
    }

    for (auto &instr : phiInstrs) {
      flattenedInstrs.push_back(instr);
    }

    for (auto &instr : jumpInstrs) {
      flattenedInstrs.push_back(instr);
    }
  }

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
    assert(this->flattenedInstrs.size() == 0);
    instrs.at(index) = Instr(args...);
  }

  void replaceInstr(size_t index, Instr instr) {
    assert(this->flattenedInstrs.size() == 0);
    instrs.at(index) = std::move(instr);
  }

  template<typename... Args>
  void replaceFlattenedInstr(size_t index, Args &&... args) {
    assert(this->flattenedInstrs.size() > 0);
    flattenedInstrs.at(index) = Instr(args...);
  }

  void replaceFlattenedInstr(size_t index, Instr instr) {
    assert(this->flattenedInstrs.size() > 0);
    flattenedInstrs.at(index) = std::move(instr);
  }

  void removeInstr(size_t index) {
    assert(this->flattenedInstrs.size() == 0);
    this->instrs.erase(this->instrs.begin() + index);
  }

  void removeFlattenedInstr(size_t index) {
    assert(this->flattenedInstrs.size() > 0);
    this->flattenedInstrs.erase(this->flattenedInstrs.begin() + index);
  }

  void write(AsmWriter &writer) const {
    writer.writeLabel(getBlockLabel(node));

    for (auto &instr : flattenedInstrs) {
      writer.writeInstr(instr);
    }
  }

  const std::string &getComment() const { return comment; }
  ir_node *getNode() { return node; }
  std::string getLabelStr() { return getBlockLabel(this->node); }
};

class Function {
  std::string fnName;
  int ARsize = 0;

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
  int getARSize() { return ARsize; }

  void writeProlog(AsmWriter &writer) const;
  void writeEpilog(AsmWriter &writer) const;

  void write(AsmWriter &writer) const;
};

struct Program {
  std::vector<Function> functions;

public:
  void flattenFunctions() {
    for (auto &f : functions) {
      for (auto &b : f.orderedBasicBlocks) {
        b->flattenInstrs();
      }
    }
  }
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
