#include "eval.hh"
#include "state.hh"
#include "jit.hh"

int eval(Prog& p) {
  State s = State(p);
  return s.run_state();
}

int eval_jit(Prog& p) {
  JIT j = JIT(p);
  return j.run_jit();
}
