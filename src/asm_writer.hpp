#ifndef ASM_WRITER_H
#define ASM_WRITER_H

#include <ostream>

class AsmWriter {
  std::ostream &out;
public:
  AsmWriter(std::ostream &o) : out(o) {}
  void write() {
    writeTextSection();
  }

private:
  void writeTextSection();

  void writeInstruction(const std::string& instr) {
    out << '\t' << instr << '\n';
  }
  void writeLabel(const std::string& label) {
    out << label << '\n';
  }
};

#endif // ASM_WRITER_H
