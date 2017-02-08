#ifndef STATE_HH
#define STATE_HH
#include <vector>
#include <map>
#include "ast.hh"

// A value can be either a Heap address or a primitive integer
struct Value
{
  bool isAddr;
  int i;
};

enum Code {EVAL, ENTER, RETURNCON, RETURNINT};
class Closure {
public:
  // A closure is if of the form (vs @flag xs -> e) ws
  LambdaForm lf;
  std::map<std::string, int> env;

  Closure(LambdaForm lf, std::map<std::string, int> env): lf(lf), env(env){}
};

class State {
public:

  State(Prog& p);
  // The code state
  Code code;

  // Eval
  // Evaluate the expression e in environment p
  // and apply its value to the arguments on the
  // argument stack
  Expr* eval_expr;
  std::map<std::string, int> eval_env;

  // Enter Arg
  // Apply the closure at address a to the arguments on the
  // arugments stack
  int enter_addr;

  // ReturnCons c ws
  // Return the constructor c applied to values ws to the continuations
  // on the return stack
  Constr* return_constr;
  std::vector<Value> constr_values;

  // ReturnInt k
  // Return the primitive interger k to the continuation on the return
  // stack
  int ret_k;

  // The argument state which contains values
  std::vector<Value> as;

  // The return stack which contains continuations
  std::vector<std::pair<Alt*, std::map<std::string, int>>> rs;

  // The update stack which contains update frames
  std::vector<int> us;

  // The heap, h which contains closures
  std::vector<Closure> h;

  // The global environment which gives the addresses of all closures defined
  // At the top level
  std::map<std::string, int> genv;

  // Applies function application to the state
  void app();

  // Picks the next_state and transitions to it
  void next_state();
};
#endif
