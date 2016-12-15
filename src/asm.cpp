#include "asm.hpp"

void Asm::AsmWriter::writeTextSection()
{
  writeText(".text");
}
