#include <sstream>
#include <cstdint>
#include <cstring>

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
  // do init work ...
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  const char* char_data = reinterpret_cast<const char*>(data);
  std::istringstream input(std::string(char_data, size));

  // TEST
  if (size > 0 && data[0] == 'H') {
    if (size > 1 && data[1] == 'I') {
      if (size > 2 && data[2] == '!') {
        __builtin_trap();
      }
    }
  }

  return 0; // Non-zero return values are reserved for future use.
}
