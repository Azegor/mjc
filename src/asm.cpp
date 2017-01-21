#include "asm.hpp"
#include <cstring>

namespace Asm {



const char *getRegAsmName(const RegName name, const RegMode mode) {
  switch (mode) {
  case RegMode::R: {
    switch(name) {
    case RegName::ax : return "%rax";
    case RegName::bx : return "%rbx";
    case RegName::cx : return "%rcx";
    case RegName::dx : return "%rdx";
    case RegName::bp : return "%rbp";
    case RegName::sp : return "%rsp";
    case RegName::si : return "%rsi";
    case RegName::di : return "%rdi";
    case RegName::r8 : return "%r8";
    case RegName::r9 : return "%r9";
    case RegName::r10: return "%r10";
    case RegName::r11: return "%r11";
    case RegName::r12: return "%r12";
    case RegName::r13: return "%r13";
    case RegName::r14: return "%r14";
    case RegName::r15: return "%r15";
    }
  }
  case RegMode::E: {
    switch(name) {
    case RegName::ax : return "%eax";
    case RegName::bx : return "%ebx";
    case RegName::cx : return "%ecx";
    case RegName::dx : return "%edx";
    case RegName::bp : return "%ebp";
    case RegName::sp : return "%esp";
    case RegName::si : return "%esi";
    case RegName::di : return "%edi";
    case RegName::r8 : return "%r8d";
    case RegName::r9 : return "%r9d";
    case RegName::r10: return "%r10d";
    case RegName::r11: return "%r11d";
    case RegName::r12: return "%r12d";
    case RegName::r13: return "%r13d";
    case RegName::r14: return "%r14d";
    case RegName::r15: return "%r15d";
    }
  }
  case RegMode::L: {
    switch(name) {
    case RegName::ax : return "%al";
    case RegName::bx : return "%bl";
    case RegName::cx : return "%cl";
    case RegName::dx : return "%dl";
    case RegName::bp :
    case RegName::sp :
    case RegName::si :
    case RegName::di : assert(false);
    case RegName::r8 : return "%r8l";
    case RegName::r9 : return "%r9l";
    case RegName::r10: return "%r10l";
    case RegName::r11: return "%r11l";
    case RegName::r12: return "%r12l";
    case RegName::r13: return "%r13l";
    case RegName::r14: return "%r14l";
    case RegName::r15: return "%r15l";
    }
  }
  default:
    assert(false);
  }
}

RegMode getRegMode(ir_node *node) {
  ir_mode *mode = get_irn_mode(node);

  if (mode == mode_Is) {
    return RegMode::E;
  }
  if (mode == mode_P) {
    return RegMode::R;
  }
  if (mode == mode_Bu) {
    return RegMode::L;
  }
  if (mode == mode_T) {
    return RegMode::R;
  }
  if (mode == mode_Ls) {
    return RegMode::R;
  }
  ir_printf("Invalid node mode %m for node %n %N\n", mode, node, node);
  assert(0);
}

std::ostream &operator<<(std::ostream &o, const Op &op) {
  switch(op.type) {
    case OP_IMM:
      o << '$' << op.p.imm.value;
      break;
    case OP_REG:
      o << getRegAsmName(op.p.reg.name, op.p.reg.mode);
      break;
    case OP_IND:
      if (op.p.ind.offset != 0)
        o << op.p.ind.offset << '(' << getRegAsmName(op.p.ind.base, op.p.ind.mode) << ')';
      else
        o << '(' << getRegAsmName(op.p.ind.base, op.p.ind.mode) << ')';
    break;
      case OP_STR:
      o << *(op.p.str.str);
    break;
    default:
    assert(false);
  }

  return o;
}

std::ostream &operator<<(std::ostream &o, const Instr &instr) {
  if (instr.mnemonic->opcode == Opcode::Label) {
    // *Tiny* hack.
    o << instr.ops[0] << ':';
    return o;
  }

  o << instr.mnemonic->name;

  if (instr.nOps > 0) {
    o << ' ' << instr.ops[0];
    if (instr.nOps > 1) {
      o << ", " << instr.ops[1];
    }
  }

  if (instr.comment.size() > 0) {
    o << " /* " << instr.comment << " */";
  }

  return o;
}

Instr makeJump(std::string target, ir_relation relation) {
  const Mnemonic *mnemonic;
  switch(relation) {
    case ir_relation_equal:
      mnemonic = &Je;
      break;
    case ir_relation_less_greater:
      mnemonic = &Jne;
      break;
    case ir_relation_true:
      mnemonic = &Jmp;
      break;
    case ir_relation_greater:
      mnemonic = &Jg;
      break;
    case ir_relation_greater_equal:
      mnemonic = &Jge;
      break;
    case ir_relation_less:
      mnemonic = &Jl;
      break;
    case ir_relation_less_equal:
      mnemonic = &Jle;
      break;
    default:
      assert(false);
  }

  return Instr(mnemonic, Op(std::move(target)));
}

Instr makeMov(const RegMode mode, Op source, Op dest, std::string comment) {
  switch (mode) {
    case RegMode::R:
      return Instr(&Movq, std::move(source), std::move(dest), comment);
    case RegMode::E:
      return Instr(&Movl, std::move(source), std::move(dest), comment);
    case RegMode::L:
      return Instr(&Movb, std::move(source), std::move(dest), comment);
    default:
      assert(false);
  }
}

Op rax() {
  return Op(RegName::ax, RegMode::R);
}

Op rbx() {
  return Op(RegName::bx, RegMode::R);
}

Op rcx() {
  return Op(RegName::cx, RegMode::R);
}

Op rbp() {
  return Op(RegName::bp, RegMode::R);
}

Op rsp() {
  return Op(RegName::sp, RegMode::R);
}



std::string getBlockLabel(ir_node *node) {
  assert(is_Block(node));

  return ".L" + std::to_string(get_irn_node_nr(node));
}


void AsmWriter::writeTextSection() { writeText(".text"); }

// --- Function ---

void Function::writeProlog(AsmWriter &writer) const {
  // TODO do the "right way" with writeInstruction!
  writer.writeString("pushq %rbp");
  writer.writeString("mov %rsp, %rbp");
  if (ARsize > 0)
    writer.writeString("subq $" + std::to_string(ARsize) + ", %rsp");
  writer.writeLabel('.' + fnName + "_body");
}
void Function::writeEpilog(AsmWriter &writer) const {
  writer.writeLabel('.' + this->getEpilogLabel());
  if (ARsize > 0)
    writer.writeString("addq $" + std::to_string(ARsize) + ", %rsp");
  writer.writeString("leave");
  writer.writeString("ret");
}

void Function::write(AsmWriter &writer) const {
  const std::string &name = fnName;
  std::stringstream ss;

  // write function prolog
  ss << "Begin " << name;
  writer.writeComment(ss.str());

  writer.writeText("\t.p2align 4,,15");
  ss.str("");
  ss << "\t.globl " << name;
  writer.writeText(ss.str());

  ss.str("");
  ss << "\t.type " << name << ", @function";
  writer.writeText(ss.str());

  writer.writeLabel(fnName);

  writeProlog(writer);

  // write function content (BBs)

  for (auto &bb: orderedBasicBlocks) {
    bb->write(writer);
  }

  writeEpilog(writer);

  // write function epilog

  ss.str("");
  ss << "\t.size " << name << ", .-" << name;
  writer.writeText(ss.str());

  ss.str("");
  ss << "End " << name;
  writer.writeComment(ss.str());
  writer.writeText(""); // newline
}
}
