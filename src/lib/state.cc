#include "state.hh"
#include <sstream>
#include <iostream>

using namespace std;

State::State(Prog& p){
  // To start the state, we should evaluate main, as,rs, us, should be empty
  // and we should add all the definitions to globals and the corresponding
  // closures to the heap
  as = vector<Value>();
  rs = vector<pair<Alt*, map<string, Value>>>();
  us = vector<int>();
  h = vector<Closure>();
  genv  = map<string, Value>();

  // First get a list of all the names
  auto binds = vector<string>();
  auto lambdas = vector<LambdaForm>();
  for (auto& bind : *(p.binds)) {
    binds.push_back(bind->v->s);
    lambdas.push_back(*(bind->lf));
  }

  // Insert all the addresses into the map
  for (int i = 0; i< binds.size(); i++) {
    Value v = {true, i};
    genv.insert(pair<string,Value>(binds[i], v));
  }

  for (auto& lambda : lambdas) {
    h.push_back(Closure(lambda, genv));
  }

  code = EVAL;
  auto main_closure = h[genv.find("main")->second.i];
  eval_expr = main_closure.lf.e;
  eval_env = genv;
}


Value val(map<string, Value> local, map<string, Value> global, Atom* a) {
  if (Var* t = dynamic_cast<Var*>(a)) {
    auto it = local.find(t->s);
    if (it != local.end()) {
      return it->second;
    }

    auto it2 = global.find(t->s);
    if (it2 != global.end()) {
      return it2->second;
    }
    throw std::runtime_error("Could not find variable");
  } else if (Literal* t = dynamic_cast<Literal*>(a)) {
    return Value{false, t->i};
  } else {
    throw std::runtime_error("Could not cast atom");
  }
}

int State::run_state(){
  while (1){
    switch (code) {
    case EVAL:
      // the expr is is a function application
      if (App* a = dynamic_cast<App*>(eval_expr)) {
        Value v = val(eval_env, genv, a->function_name);
        if (v.isAddr) {
          code = ENTER;
          enter_addr = v.i;
          // Push the arguments onto the arg stack
          for (auto it = a->args->rbegin(); it!= a->args->rend(); ++it) {
            as.push_back(val(eval_env, genv, *it));
          }
        } else {
          code = RETURNINT;
          ret_k = v.i;
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
          Value v = {true, addr};
          h.push_back(c);
          eval_env.insert(pair<string,Value>(bind->v->s, v));
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
          Value v = {true, addr++};
          eval_env.insert(pair<string,Value>(var, v));
        }

        // add all the binds to the heap
        for (auto& bind: *(l->binds)) {
          Closure c = Closure(*(bind->lf), eval_env);
          int addr = h.size();
          h.push_back(c);
        }
      } else if (Case* c = dynamic_cast<Case*>(eval_expr)) {
        eval_expr = c->e;
        rs.push_back(pair<Alt*, std::map<std::string, Value>>(c->a, eval_env));
      } else if (SatConstr* c = dynamic_cast<SatConstr*>(eval_expr)) {
        code = RETURNCON;
        return_constr = c->c;
        constr_values = vector<Value>();
        for (auto& atom: *(c->a)) {
          constr_values.push_back(val(eval_env, genv, atom));
        }
      } else if (Lit* l = dynamic_cast<Lit*>(eval_expr)) {
        code = RETURNINT;
        ret_k = l->l->i;
      } else if (SatOp* s = dynamic_cast<SatOp*>(eval_expr)) {
        Value v1 = val(eval_env, genv, (*(s->a))[0]);
        Value v2 = val(eval_env, genv, (*(s->a))[1]);
        if (v1.isAddr || v2.isAddr) {
          throw std::runtime_error("Arguments to primitive operator were not ints");
        }
        int x1 = v1.i;
        int x2 = v2.i;
        code = RETURNINT;
        switch(s->p) {
        case ADD:
          ret_k = x1+x2;
          break;
        case SUB:
          ret_k = x1-x2;
          break;
        case MUL:
          ret_k = x1*x2;
          break;
        case DIV:
          if (x2 == 0) {
            throw std::runtime_error("Divide by 0 error");
          }
          ret_k = x1/x2;
          break;
        }
      }
      break;
    case ENTER:
      {
        Closure clos = h[enter_addr];
        LambdaForm lf = clos.lf;
        // For non updatable closures
        //if (!lf.u->update) {
        // For now, don't update any closures
        if (true) {
          code = EVAL;
          eval_expr = lf.e;
          eval_env = map<string, Value>();
          for (auto& var : *(lf.v1)) {
            eval_env.insert(pair<string, Value>(var->s, genv.find(var->s)->second));
          }
          for (auto& var : *(lf.v2)) {
            Value v = as.back();
            as.pop_back();
            eval_env.insert(pair<string, Value>(var->s, v));
          }
        }
      }
      break;
    case RETURNCON:
      {
        pair<Alt*, std::map<std::string, Value>> top = rs.back();
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
            eval_env.insert(pair<string, Value>((*(match->v))[i]->s, constr_values[i]));
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
            map<string, Value> arb_env;
            for (int i = 0; i < num_bars; i++) {
              stringstream ss;
              ss << "x" << i;
              string s = ss.str();
              arb_vars->push_back(new Var(s));
              arb_env.insert(pair<string,Value>(s, constr_values[i]));
            }
            vector<Atom*>* arb_atoms = new vector<Atom*>();
            for (auto& var : *arb_vars) {
              arb_atoms->push_back(var);
            }

            LambdaForm lf = LambdaForm(arb_vars, new UpdateFlag(false), new vector<Var*>(), new SatConstr(return_constr, arb_atoms));
            Closure constr = Closure(lf, arb_env);
            int addr = h.size();
            Value v = {true, addr};
            h.push_back(constr);
            eval_env.insert(pair<string, Value>(n->v->s, v));
          }
        }

        break;
      }
    case RETURNINT:
      {
        // If there is nothing on the return stack, we are done
        if (rs.empty()) {
          return ret_k;
        }
        pair<Alt*, std::map<std::string, Value>> top = rs.back();
        rs.pop_back();
        PAlts* palts = dynamic_cast<PAlts*>(top.first);
        PAlt* match = NULL;
        for (auto& palt: *(palts->v)) {
          if (palt->l->i == ret_k) {
            match = palt;
            break;
          }
        }
        // If match is not NULL take it, else use the default
        if (match) {
          code = EVAL;
          eval_expr = match->e;
        } else {
          // default
          Dflt* dflt = palts->d;
          // if there is not variable bound, just set the expression
          // If there is, we need to allocate a constructor closure to bind to it
          if (Unnamed* u = dynamic_cast<Unnamed*>(u)) {
            code = EVAL;
            eval_expr = u->e;
          } else if (Named* n = dynamic_cast<Named*>(u)) {
            code = EVAL;
            eval_expr = u->e;
            Value v = {false, ret_k};
            eval_env.insert(pair<string, Value>(n->v->s, v));
          }
        }

        break;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Value &v){
  os << (v.isAddr ? "Addr: " : "Prim: ") << v.i;
}
