#ifndef ASM_WRITER_H
#define ASM_WRITER_H

#include <ostream>

#include "asm.hpp"

class AsmWriter {
  std::ostream &out;
public:
  AsmWriter(std::ostream &o) : out(o) {}
  void write() {
    writeTextSection();
  }

  void writeTextSection();

  void writeInstruction(const std::string& instr) {
    out << '\t' << instr << '\n';
  }
  void writeLabel(const std::string& label) {
    out << label << '\n';
  }

  void writeComment(const std::string &comment) {
    out << "/* -- " << comment << " */\n";
  }
};

#endif // ASM_WRITER_H
