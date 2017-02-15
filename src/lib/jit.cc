#include "jit.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

JIT::JIT(Prog& p): p(p){}

int JIT::run_jit(){
  // Machine code for
  // mov eax, 0
  // ret
  unsigned char code[] = {0xb8, 0x01, 0x00, 0x00, 0x00, 0xc3};
  void *mem = mmap(NULL, sizeof(code), PROT_WRITE | PROT_EXEC,
                   MAP_ANON | MAP_PRIVATE, -1, 0);
  memcpy(mem, code, sizeof(code));

  // The function will return the user's value.
  return ((int (*)(void)) mem)();
}
