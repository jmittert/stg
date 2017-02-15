#ifndef ASM_HH
#define ASM_HH
#include <vector>
#include <string>

class AsmBuilder {
public:
  std::vector<unsigned char> code;
  friend AsmBuilder& operator<<(AsmBuilder& os, std::string&e);
  unsigned char* get_code();
};
#endif
