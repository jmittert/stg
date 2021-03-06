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

  friend std::ostream& operator<<(std::ostream& os, const Value &v);
};

enum Code {EVAL, ENTER, RETURNCON, RETURNINT};
class Closure {
public:
  // A closure is if of the form (vs @flag xs -> e) ws
  LambdaForm lf;
  std::vector<Value> free_vars;

  Closure(LambdaForm lf, std::vector<Value> free_vars): lf(lf), free_vars(free_vars){}
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
  std::shared_ptr<const Expr> eval_expr;
  std::map<std::string, Value> eval_env;

  // Enter Arg
  // Apply the closure at address a to the arguments on the
  // arugments stack
  int enter_addr;

  // ReturnCons c ws
  // Return the constructor c applied to values ws to the continuations
  // on the return stack
  Constr return_constr = Constr("");
  std::vector<Value> constr_values;

  // ReturnInt k
  // Return the primitive interger k to the continuation on the return
  // stack
  int ret_k;

  // The argument state which contains values
  std::vector<Value> as;

  // The return stack which contains continuations
  std::vector<std::pair<std::shared_ptr<const Alt>, std::map<std::string, Value>>> rs;

  // The update stack which contains update frames
  // A frame is a triple of the arugment stack, the return stack and the
  // closure being entered
  std::vector<std::tuple<std::vector<Value>, std::vector<std::pair<std::shared_ptr<const Alt>, std::map<std::string, Value>>>, std::shared_ptr<Closure>>> us;

  // The heap, h which contains closures
  std::vector<Closure> h;

  // The global environment which gives the addresses of all closures defined
  // At the top level
  std::map<std::string, Value> genv;

  // Runs the state to completion
  int run_state();
};

std::string printEnv(std::map<std::string, Value>& m);
#endif
