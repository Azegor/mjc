#include "asm_writer.hpp"

void AsmWriter::writeTextSection()
{
  writeInstruction(".text");
}
