#include "asm.hpp"

namespace Asm {

  const char* X86_64Register::getAsmName() const {
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

void AsmWriter::writeTextSection()
{
  writeText(".text");
}

void Function::write(AsmWriter &writer) const {
  const std::string &name = fnName.name;
  std::stringstream ss;

  // write function prolog
  ss << "Begin " << name;
  writer.writeComment(ss.str());

  writer.writeText(".p2align 4,,15");
  ss.str("");
  ss << ".globl " << name;
  writer.writeText(ss.str());

  ss.str("");
  ss << ".type " << name << ", @function";
  writer.writeText(ss.str());

  writer.writeLabel(fnName);

  // write function content (BBs)

  for (auto &bb : basicBlocks) {
    bb.write(writer);
  }

  // write function epilog

  ss.str("");
  ss << ".size " << name << ", .-" << name;
  writer.writeText(ss.str());

  ss.str("");
  ss << "End " << name;
  writer.writeComment(ss.str());
  writer.writeText(""); // newline
}

}