#ifndef JIT_HH
#define JIT_HH
#include "ast.hh"

class JIT {
public:
  JIT(Prog& p);
  // Run the program with JIT
  int run_jit();
private:
  Prog& p;
};
#endif
