#include "state.hh"
#include <sstream>

using namespace std;

State::State(Prog& p){
  // To start the state, we should evaluate main, as,rs, us, should be empty
  // and we should add all the definitions to globals and the corresponding
  // closures to the heap
  as = vector<Value>();
  rs = vector<std::pair<Alt*, std::map<std::string, int>>>();
  us = vector<int>();
  h = vector<Closure>();
  genv  = map<string, int>();

  // First get a list of all the names
  auto binds = vector<string>();
  auto lambdas = vector<LambdaForm>();
  for (auto& bind : *(p.binds)) {
    binds.push_back(bind->v->s);
    lambdas.push_back(*(bind->lf));
  }

  // Insert all the addresses into the map
  for (int i = 0; i< binds.size(); i++) {
    genv.insert(pair<string,int>(binds[i], i));
  }

  for (auto& lambda : lambdas) {
    h.push_back(Closure(lambda, genv));
  }

  code = EVAL;
  auto main_closure = h[genv.find("main")->second];
  eval_expr = main_closure.lf.e;
  eval_env = genv;
}


Value val(map<string, int> local, map<string, int> global, Atom* a) {
  if (Var* t = dynamic_cast<Var*>(a)) {
    auto it = local.find(t->s);
    if (it != local.end()) {
      Value v = {false, it->second};
      return v;
    }

    auto it2 = global.find(t->s);
    if (it != global.end()) {
      Value v = {false, it->second};
      return v;
    }
    throw "Could not find variable";
  } else if (Literal* t = dynamic_cast<Literal*>(a)) {
    return Value{false, t->i};
  } else {
    throw "Could not cast atom";
  }
}

void State::app(){
}

void State::next_state(){
  switch (code) {
  case EVAL:
    // the expr is is a function application
    if (App* a = dynamic_cast<App*>(eval_expr)) {
      Value v = val(eval_env, genv, a->v);
      if (v.isAddr) {
        code = ENTER;
        enter_addr = v.i;
        // Push the arguments onto the arg stack
        for (auto it = a->a->rbegin(); it!= a->a->rend(); ++it) {
          as.push_back(val(eval_env, genv, *it));
        }
      }
      // Let definition
    } else if (LocalDef* l = dynamic_cast<LocalDef*>(eval_expr)) {
      code = EVAL;
      eval_expr = l->e;
      auto pre_env = eval_env;
      // add all the binds to the scope
      for (auto& bind: *(l->binds)) {
        Closure c = Closure(*(bind->lf), pre_env);
        int addr = h.size();
        h.push_back(c);
        eval_env.insert(pair<string,int>(bind->v->s, addr));
      }
      // Let rec binding
    } else if (LocalRec* l = dynamic_cast<LocalRec*>(eval_expr)) {
      // Similar to the let binding, but each closure get also can refer to
      // the other bindings as well
      code = EVAL;
      eval_expr = l->e;

      // First get a list of all the names
      auto vars = vector<string>();
      auto lambdas = vector<LambdaForm>();
      for (auto& bind : *(l->binds)) {
        vars.push_back(bind->v->s);
        lambdas.push_back(*(bind->lf));
      }

      int addr = h.size();
      // Insert all the addresses into the map
      for (auto& var : vars) {
        eval_env.insert(pair<string,int>(var, addr++));
      }

      // add all the binds to the heap
      for (auto& bind: *(l->binds)) {
        Closure c = Closure(*(bind->lf), eval_env);
        int addr = h.size();
        h.push_back(c);
      }
    } else if (Case* c = dynamic_cast<Case*>(eval_expr)) {
      eval_expr = c->e;
      rs.push_back(pair<Alt*, std::map<std::string, int>>(c->a, eval_env));
    } else if (SatConstr* c = dynamic_cast<SatConstr*>(eval_expr)) {
      code = RETURNCON;
      return_constr = c->c;
      constr_values = vector<Value>();
      for (auto& atom: *(c->a)) {
        constr_values.push_back(val(eval_env, genv, atom));
      }
    }
    break;
  case ENTER:
    {
      Closure clos = h[enter_addr];
      LambdaForm lf = clos.lf;
      // For non updatable closures
      if (!lf.u->update) {
        code = EVAL;
        eval_expr = lf.e;
        eval_env = map<string, int>();
        for (auto& var : *(lf.v1)) {
          eval_env.insert(pair<string, int>(var->s, genv.find(var->s)->second));
        }
        for (auto& var : *(lf.v2)) {
          Value v = as.back();
          as.pop_back();
          if (!v.isAddr) {
            throw "Value was an int, guess you do have to fix it";
          }
          eval_env.insert(pair<string, int>(var->s, v.i));
        }
      }
    }
    break;
  case RETURNCON:
    {
      pair<Alt*, std::map<std::string, int>> top = rs.back();
      rs.pop_back();
      AAlts* aalts = dynamic_cast<AAlts*>(top.first);
      AAlt* match = NULL;
      for (auto& aalt: *(aalts->v)) {
        if (aalt->c->s == return_constr->s) {
          match = aalt;
          break;
        }
      }
      // If match is not NULL take it, else use the default
      if (match) {
        code = EVAL;
        eval_expr = match->e;
        // Assign the variables to the values of the actual arguments
        for (int i = 0; i< constr_values.size(); ++i) {
          eval_env.insert(pair<string, int>((*(match->v))[i]->s, constr_values[i].i));
        }
      } else {
        //default
        Dflt* dflt = aalts->d;
        // if there is not variable bound, just set the expression
        // If there is, we need to allocate a constructor closure to bind to it
        if (Unnamed* u = dynamic_cast<Unnamed*>(u)) {
          code = EVAL;
          eval_expr = u->e;
        } else if (Named* n = dynamic_cast<Named*>(u)) {
            code = EVAL;
            eval_expr = u->e;
            // Create a list of arbitrary vars to match the addresses we have
            int num_bars = constr_values.size();
            vector<Var*>* arb_vars = new vector<Var*>();
            map<string, int> arb_env;
            for (int i = 0; i < num_bars; i++) {
              stringstream ss;
              ss << "x" << i;
              string s = ss.str();
              arb_vars->push_back(new Var(s));
              arb_env.insert(pair<string,int>(s, constr_values[i].i));
            }
            vector<Atom*>* arb_atoms = new vector<Atom*>();
            for (auto& var : *arb_vars) {
              arb_atoms->push_back(var);
            }

            LambdaForm lf = LambdaForm(arb_vars, new UpdateFlag(false), new vector<Var*>(), new SatConstr(return_constr, arb_atoms));
            Closure constr = Closure(lf, arb_env);
            int index = h.size();
            h.push_back(constr);
            eval_env.insert(pair<string, int>(n->v->s, index));
        }
      }

    break;
    }
  case RETURNINT:
    break;
  defaut:
    break;
  }
}
