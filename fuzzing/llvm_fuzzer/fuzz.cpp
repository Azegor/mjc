#include <sstream>
#include <cstdint>
#include <cstring>

#include "../../src/compiler.hpp"

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
  // do init work ...
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  const char* char_data = reinterpret_cast<const char*>(data);
  std::istringstream input(std::string(char_data, size));

  CompilerOptions options;
//   options.fuzzLexer = true; // Modify Options here by hand
//   options.fuzzParser = true; // Modify Options here by hand
  options.fuzzSemantic = true; // Modify Options here by hand
  options.inputFileName = "<internal>";
  Compiler compiler{InputFile{"<internal>", &input}, options};

  try {
    compiler.run(); // ignore returncode (valid from a fuzzing perspective)
  }
  catch(CompilerError &e){
    // ignore all compiler-intern exceptions
  }
  catch (...) {
    __builtin_trap();
  }


  return 0; // Non-zero return values are reserved for future use.
}
