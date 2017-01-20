#include "asm.hpp"

namespace Asm {

X86Reg X86Reg::noReg;

const char *X86Reg::getAsmName() const {
  switch (mode) {
  case Mode::R: {
    switch(name) {
    case Name::none: assert(false);
    case Name::ax : return "%rax";
    case Name::bx : return "%rbx";
    case Name::cx : return "%rcx";
    case Name::dx : return "%rdx";
    case Name::bp : return "%rbp";
    case Name::sp : return "%rsp";
    case Name::si : return "%rsi";
    case Name::di : return "%rdi";
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
  case Mode::E: {
    switch(name) {
    case Name::none: assert(false);
    case Name::ax : return "%eax";
    case Name::bx : return "%ebx";
    case Name::cx : return "%ecx";
    case Name::dx : return "%edx";
    case Name::bp : return "%ebp";
    case Name::sp : return "%esp";
    case Name::si : return "%esi";
    case Name::di : return "%edi";
    case Name::r8 : return "%r8d";
    case Name::r9 : return "%r9d";
    case Name::r10: return "%r10d";
    case Name::r11: return "%r11d";
    case Name::r12: return "%r12d";
    case Name::r13: return "%r13d";
    case Name::r14: return "%r14d";
    case Name::r15: return "%r15d";
    }
  }
  case Mode::L: {
    switch(name) {
    case Name::none: assert(false);
    case Name::ax : return "%al";
    case Name::bx : return "%bl";
    case Name::cx : return "%cl";
    case Name::dx : return "%dl";
    case Name::bp :
    case Name::sp :
    case Name::si :
    case Name::di : assert(false);
    case Name::r8 : return "%r8l";
    case Name::r9 : return "%r9l";
    case Name::r10: return "%r10l";
    case Name::r11: return "%r11l";
    case Name::r12: return "%r12l";
    case Name::r13: return "%r13l";
    case Name::r14: return "%r14l";
    case Name::r15: return "%r15l";
    }
  }
  default:
    // TODO
    assert(false);
  }
}

Asm::X86Reg::Mode X86Reg::getRegMode(ir_node *node) {
  ir_mode *mode = get_irn_mode(node);

  if (mode == mode_Is) {
    return Mode::E;
  }
  if (mode == mode_P) {
    return Mode::R;
  }
  if (mode == mode_Bu) {
    return Mode::L;
  }
  if (mode == mode_T) {
    return Mode::R;
  }
  if (mode == mode_Ls) {
    return Mode::R;
  }
  ir_printf("Invalid node mode %m for node %n %N\n", mode, node, node);
  assert(0);
}

std::ostream &operator<<(std::ostream &o, const Asm::X86Reg::Mode mode) {
  using Asm::X86Reg;
  switch(mode) {
    case X86Reg::Mode::None: o << "None"; break;
    case X86Reg::Mode::R:    o << "R"; break;
    case X86Reg::Mode::E:    o << "E"; break;
    case X86Reg::Mode::_:    o << "_"; break;
    case X86Reg::Mode::H:    o << "H"; break;
    case X86Reg::Mode::L:    o << "L"; break;
  }
  return o;
}

std::string getBlockLabel(ir_node *node) {
  assert(is_Block(node));

  return "L" + std::to_string(get_irn_node_nr(node));
}


void AsmWriter::writeTextSection() { writeText(".text"); }

// --- Function ---

void Function::writeProlog(AsmWriter &writer) const {
  // TODO do the "right way" with writeInstruction!
  writer.writeString("pushq %rbp");
  writer.writeString("mov %rsp, %rbp");
  if (ARsize > 0)
    writer.writeString("subq $" + std::to_string(ARsize) + ", %rsp");
  writer.writeLabel('.' + fnName.name + "_body");
}
void Function::writeEpilog(AsmWriter &writer) const {
  writer.writeLabel('.' + this->getEpilogLabel());
  if (ARsize > 0)
    writer.writeString("addq $" + std::to_string(ARsize) + ", %rsp");
  writer.writeString("leave");
  writer.writeString("ret");
}

void Function::write(AsmWriter &writer) const {
  const std::string &name = fnName.name;
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
