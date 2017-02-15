#ifndef EVAL_HH
#define EVAL_HH

#include "ast.hh"

int eval(Prog& p);
int eval_jit(Prog& p);
#endif
