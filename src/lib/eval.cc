#include "eval.hh"
#include "state.hh"

int eval(Prog& p) {
  State s = State(p);
  return s.run_state();
}
