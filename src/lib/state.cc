#include "state.hh"
#include <sstream>
#include <iostream>

using namespace std;

State::State(Prog& p){
  // To start the state, we should evaluate main, as,rs, us, should be empty
  // and we should add all the definitions to globals and the corresponding
  // closures to the heap
  as = vector<Value>();
  rs = vector<pair<shared_ptr<const Alt>, map<string, Value>>>();
  us = vector<tuple<vector<Value>, vector<pair<shared_ptr<const Alt>, map<string, Value>>>, shared_ptr<Closure> >>();
  h = vector<Closure>();
  genv  = map<string, Value>();

  // First get a list of all the names
  auto binds = vector<string>();
  auto lambdas = vector<LambdaForm>();
  for (auto& bind : p.binds) {
    binds.push_back(bind.v.s);
    lambdas.push_back(bind.lf);
  }

  // Insert all the addresses into the map
  for (int i = 0; i < (int)binds.size(); i++) {
    Value v = {true, i};
    genv[binds[i]] = v;
  }

  for (auto& lambda : lambdas) {
    vector<Value> free_vars;
    // look up each free bar in the lambda
    for (auto& var : lambda.v1) {
      Value v = genv.find(var.s)->second;
      free_vars.push_back(v);
    }
    h.push_back(Closure(lambda, free_vars));
  }

  // Set up the initial conditions
  // We want to evaluate main with nothing in the environment
  // except the globals
  code = EVAL;
  auto main_closure = h[genv.find("main")->second.i];
  eval_expr = main_closure.lf.e;
  eval_env = map<string, Value>();
}

std::string printEnv(map<string, Value>& m) {
  stringstream ss;
  ss << "{\n";
  for (auto& pair : m) {
    ss << pair.first << ": " << pair.second << endl;
  }
  ss << "}\n";
  return ss.str();
}

Value val(map<string, Value> local, map<string, Value> global, const Atom* a) {
  if (auto t = dynamic_cast<const Var*>(a)) {
    auto it = local.find(t->s);
    if (it != local.end()) {
      return it->second;
    }

    auto it2 = global.find(t->s);
    if (it2 != global.end()) {
      return it2->second;
    }
    cout << "local: " << printEnv(local) << endl;
    cout << "global: " << printEnv(global) << endl;
    throw std::runtime_error("Could not find variable: " + t->s);
  } else if (auto t = dynamic_cast<const Literal*>(a)) {
    return Value{false, t->i};
  } else {
    throw std::runtime_error("Could not cast atom");
  }
}

int State::run_state(){
  while (1){
    switch (code) {
    case EVAL:
      if (auto a = dynamic_pointer_cast<const App>(eval_expr)) {
        // Function application
        Value v = val(eval_env, genv, &(a->function_name));
        if (v.isAddr) {
          // If the name resolves to a pointer, then we have a function
          // Push the appropriate arguments onto the stack then eval it
          code = ENTER;
          enter_addr = v.i;
          // Push the arguments in backwards order so they then match
          // the function when we pop them
          for (auto it = a->args.rbegin(); it != a->args.rend(); ++it) {
            const Value v = val(eval_env, genv, it->get());
            as.push_back(v);
          }
        } else {
          // Otherwise, if the name is a primitive, prepare to return it
          // to the next continuation
          code = RETURNINT;
          ret_k = v.i;
        }
      } else if (auto l = dynamic_pointer_cast<const LocalDef>(eval_expr)) {
        code = EVAL;
        eval_expr = l->e;
        auto pre_env = eval_env;
        // add all the binds to the scope
        for (auto& bind: l->binds) {
          vector<Value> free_vars;
          // look up each free var in the lambda
          for (auto& var : bind.lf.v1) {
            Value v = pre_env.find(var.s)->second;
            free_vars.push_back(v);
          }
          Closure c = Closure(bind.lf, free_vars);
          int addr = h.size();
          Value v = {true, addr};
          h.push_back(c);
          eval_env[bind.v.s] = v;
        }
        // Let rec binding
      } else if (auto l = dynamic_pointer_cast<const LocalRec>(eval_expr)) {
        // Similar to the let binding, but each closure get also can refer to
        // the other bindings as well
        code = EVAL;
        eval_expr = l->e;

        // First get a list of all the names
        auto vars = vector<string>();
        auto lambdas = vector<LambdaForm>();
        for (auto& bind : l->binds) {
          vars.push_back(bind.v.s);
          lambdas.push_back(bind.lf);
        }

        int addr = h.size();
        // Insert all the addresses into the map
        for (auto& var : vars) {
          Value v = {true, addr++};
          eval_env[var] = v;
        }

        // add all the binds to the heap
        for (auto& bind: l->binds) {
          vector<Value> free_vars;
          // look up each free var in the lambda
          for (auto& var : bind.lf.v1) {
            Value v = eval_env.find(var.s)->second;
            free_vars.push_back(v);
          }
          Closure c = Closure(bind.lf, free_vars);
          h.push_back(c);
        }
      } else if (auto c = dynamic_pointer_cast<const Case>(eval_expr)) {
        eval_expr = c->expr;
        rs.push_back(make_pair(c->alt, eval_env));
      } else if (auto c = dynamic_pointer_cast<const SatConstr>(eval_expr)) {
        code = RETURNCON;
        return_constr = c->c;
        constr_values = vector<Value>();
        for (auto& atom: c->args) {
          constr_values.push_back(val(eval_env, genv, atom.get()));
        }
      } else if (auto l = dynamic_pointer_cast<const Lit>(eval_expr)) {
        code = RETURNINT;
        ret_k = l->l.i;
      } else if (auto s = dynamic_pointer_cast<const SatOp>(eval_expr)) {
        if (s->args.size() == 1) {
          // This must be the print operator
          Value v1 = val(eval_env, genv, s->args[0].get());
          int x1 = v1.i;
          code = RETURNINT;
          ret_k = x1;
          cout << x1 << endl;
        } else {
          Value v1 = val(eval_env, genv, s->args[0].get());
          Value v2 = val(eval_env, genv, s->args[1].get());
          if (v1.isAddr || v2.isAddr) {
            stringstream ss;
            ss << "Arguments to primitive operator were not ints" << endl;
            ss << "Prim: " << primToString(s->prim) << endl;;
            ss << "Arg1: " << v1 << endl;
            ss << "Arg2: " << v2 << endl;
            ss << "Local: " << printEnv(eval_env) << endl;
            ss << "Global: " << printEnv(genv) << endl;
            throw std::runtime_error(ss.str());
          }
          int x1 = v1.i;
          int x2 = v2.i;
          code = RETURNINT;
          switch(s->prim) {
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
            ret_k = x1 / x2;
            break;
          case GT:
            code = RETURNCON;
            return_constr = Constr(x1>x2 ? "True" : "False");
            constr_values.clear();
            break;
          case LT:
            code = RETURNCON;
            return_constr = Constr(x1<x2 ? "True" : "False");
            constr_values.clear();
            break;
          case EQ:
            code = RETURNCON;
            return_constr = Constr(x1==x2 ? "True" : "False");
            constr_values.clear();
            break;
          case LTE:
            code = RETURNCON;
            return_constr = Constr(x1<=x2 ? "True" : "False");
            constr_values.clear();
            break;
          case GTE:
            code = RETURNCON;
            return_constr = Constr(x1>=x2 ? "True" : "False");
            constr_values.clear();
            break;
          case PRINT:
            throw std::runtime_error("Passed print wrong # of args");
          }
        }
      }
      break;
    case ENTER:
      {
        Closure clos = h[enter_addr];
        LambdaForm lf = clos.lf;
        // If there are not enough arguments on the stack for the closure, we
        // need to pop an update frame to fill in the rest
        unsigned as_size = as.size();
        vector<Value> old_as = as;
        vector<Var> xs = lf.v2;
        if (xs.size() > as_size) {
          if (us.empty()) {
            stringstream ss;
            ss << "Update stack is empty. Entering: "
               << lf << "with " << old_as[0] << endl;
            throw std::runtime_error(ss.str());
          }
          auto tup = us.back();
          us.pop_back();
          // Append on the popped argument stack
          auto as_popped = get<0>(tup);
          as.insert(as.begin(), as_popped.begin(), as_popped.end());

          // The return stack must be empty, use the popped one
          rs = get<1>(tup);

          shared_ptr<Closure> clos_update = get<2>(tup);
          // The arguments that were defined
          vector<Var> xs1(xs.rbegin(), xs.rbegin()+as_size);
          // The remaining arguments
          vector<Var> xs2 = vector<Var>(xs.rbegin()+as_size, xs.rend());
          vector<Var> vs = vector<Var>();
          vs.insert(vs.end(), lf.v1.begin(), lf.v1.end());
          vs.insert(vs.end(), xs1.begin(), xs1.end());
          LambdaForm lf_updated = LambdaForm(vs, UpdateFlag(false), xs2, lf.e);
          vector<Value> free_vars = clos.free_vars;
          free_vars.insert(free_vars.end(), old_as.begin(), old_as.end());
          // Update the closure
          *clos_update = Closure(lf_updated, free_vars);
          break;
        }
        // For non updatable closures
        if (!lf.u.update) {
          code = EVAL;
          eval_expr = lf.e;
          // Insert each free variable into the environment
          for (unsigned i = 0; i < clos.free_vars.size(); i++) {
            eval_env[lf.v1[i].s] = clos.free_vars[i];
          }

          // Then insert each function argument into the environment
          // Each variable in the lambda form arguments will match one
          // to one with one on the argument stack
          for (auto& var : lf.v2) {
            Value v = as.back();
            as.pop_back();
            eval_env[var.s] = v;
          }
        } else {
          // Updatable closures
          us.push_back(make_tuple(as,rs, shared_ptr<Closure>(new Closure(clos))));
          as.clear();
          rs.clear();
          code = EVAL;
          eval_expr = lf.e;
          // An updatable closure has no arguments, so we only need to add
          // the free variables
          for (unsigned i = 0; i < clos.free_vars.size(); i++) {
            eval_env[lf.v1[i].s] = clos.free_vars[i];
          }
        }
      }
      break;
    case RETURNCON:
      {
        // If the return stack is empty, check for a possible update
        if (rs.empty()) {
          // If the update stack is empty, we're done
          if (us.empty()) {
            return constr_values[0].i;
          }
          auto tup = us.back();
          us.pop_back();
          as = get<0>(tup);
          rs = get<1>(tup);

          shared_ptr<Closure> clos = get<2>(tup);

          // Create a list of arbitrary vars to match the addresses we have
          int num_bars = constr_values.size();
          vector<Var> arb_vars = vector<Var>();
          for (int i = 0; i < num_bars; i++) {
            stringstream ss;
            ss << "$" << i;
            string s = ss.str();
            arb_vars.push_back(Var(s));
          }
          vector<unique_ptr<Atom>> arb_atoms = vector<unique_ptr<Atom>>();
          for (auto& var : arb_vars) {
            arb_atoms.push_back(unique_ptr<Atom>(new Var(var.s)));
          }

          LambdaForm lf = LambdaForm(arb_vars, UpdateFlag(false), vector<Var>(), shared_ptr<Expr>(new SatConstr(return_constr, arb_atoms)));
          // Update the closure
          *clos = Closure(lf, constr_values);
          break;
        }
        pair<shared_ptr<const Alt>, std::map<std::string, Value>> top = rs.back();
        rs.pop_back();
        auto aalts = dynamic_pointer_cast<const AAlts>(top.first);
        const AAlt* match = nullptr;
        for (auto& aalt: aalts->v) {
          if (aalt.c.s == return_constr.s) {
            match = &aalt;
            break;
          }
        }
        eval_env = top.second;
        // If a match is found, take it, else use the default
        if (match != nullptr) {
          code = EVAL;
          eval_expr = match->e;
          // Assign the variables to the values of the actual arguments
          for (unsigned i = 0; i < constr_values.size(); ++i) {
            eval_env[match->v[i].s] = constr_values[i];
          }
        } else {
          //default
          shared_ptr<const Dflt> dflt = aalts->d;
          // if there is not variable bound, just set the expression
          // If there is, we need to allocate a constructor closure to bind to it
          if (auto u = dynamic_pointer_cast<const Unnamed>(dflt)) {
            code = EVAL;
            eval_expr = u->e;
          } else if (auto n = dynamic_pointer_cast<const Named>(dflt)) {
            code = EVAL;
            eval_expr = n->e;
            // Create a list of arbitrary vars to match the addresses we have
            int num_bars = constr_values.size();
            auto arb_vars = vector<Var>();
            vector<Value> free_vars;
            for (int i = 0; i < num_bars; i++) {
              stringstream ss;
              ss << "$" << i;
              string s = ss.str();
              arb_vars.push_back(Var(s));
            }
            auto arb_atoms = vector<unique_ptr<Atom>>();
            for (auto& var : arb_vars) {
              arb_atoms.push_back(unique_ptr<Var>(new Var(var.s)));
            }

            LambdaForm lf = LambdaForm(arb_vars, UpdateFlag(false), vector<Var>(), shared_ptr<Expr>(new SatConstr(return_constr, arb_atoms)));
            Closure constr = Closure(lf, constr_values);
            int addr = h.size();
            Value v = {true, addr};
            h.push_back(constr);
            eval_env[n->v.s] = v;
          } else if (auto n = dynamic_pointer_cast<const NoDflt>(dflt)) {
            stringstream ss;
            ss << "No default matched in " << *aalts << " for " << return_constr.s << endl;
            throw std::runtime_error(ss.str());
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
        pair<shared_ptr<const Alt>, std::map<std::string, Value>> top = rs.back();
        rs.pop_back();
        auto palts = dynamic_pointer_cast<const PAlts>(top.first);
        if (palts == nullptr){
          // Try default
          auto def = dynamic_pointer_cast<const Default>(top.first);
          if (def != nullptr) {
            shared_ptr<const Dflt> dflt = def->d;
            // if there is not variable bound, just set the expression
            // If there is, we need to allocate a constructor closure to bind to it
            if (auto u = dynamic_pointer_cast<const Unnamed>(dflt)) {
              code = EVAL;
              eval_expr = u->e;
            } else if (auto n = dynamic_pointer_cast<const Named>(dflt)) {
              code = EVAL;
              eval_expr = u->e;
              Value v = {false, ret_k};
              eval_env[n->v.s] = v;
            }
            break;
          }
          auto aalts = dynamic_pointer_cast<const AAlts>(top.first);
          stringstream ss;
          ss << "Tried to match a value " << ret_k << " against a AAlts " << *aalts;
          throw std::runtime_error(ss.str());
        }
        const PAlt* match = nullptr;
        for (auto& palt: palts->v) {
          if (palt.l.i == ret_k) {
            match = &palt;
            break;
          }
        }
        // If match is not nullptr take it, else use the default
        if (match != nullptr) {
          code = EVAL;
          eval_expr = match->e;
        } else {
          // default
          shared_ptr<const Dflt> dflt = palts->d;
          // if there is not variable bound, just set the expression
          // If there is, we need to allocate a constructor closure to bind to it
          if (auto u = dynamic_pointer_cast<const Unnamed>(dflt)) {
            code = EVAL;
            eval_expr = u->e;
          } else if (auto n = dynamic_pointer_cast<const Named>(dflt)) {
            code = EVAL;
            eval_expr = n->e;
            Value v = {false, ret_k};
            eval_env[n->v.s] = v;
          } else if (auto n = dynamic_pointer_cast<const NoDflt>(dflt)) {
            stringstream ss;
            ss << "No default matched in " << *palts << " for " << ret_k << endl;
            throw std::runtime_error(ss.str());
          }

        }

        break;
      }
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Value &v){
  return os << (v.isAddr ? "Addr: " : "Prim: ") << v.i;
}
