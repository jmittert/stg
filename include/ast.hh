#ifndef AST_HH
#define AST_HH
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <memory>
/*
 * Ast.hh: AST for STG
 */


class AAlt;
class PAlt;
class Bind;
class Atom {
public:
  Atom(){};
  virtual std::string toString() const = 0;
  virtual Atom* clone() const = 0;
  friend std::ostream& operator<<(std::ostream& os, const Atom &e){
    return os << e.toString();
  }
};

class Expr {
public:
  Expr(){};
  virtual std::string toString() const = 0;
  friend std::ostream& operator<<(std::ostream& os, const Expr &l){
    return os << l.toString();
  }
};

class Alt {
public:
  Alt(){};
  virtual std::string toString() const = 0;
  friend std::ostream& operator<<(std::ostream& os, const Alt &l){
    os << l.toString();
    return os;
  }
};
class Var: public Atom{
public:
  Var(std::string s): s(s){};
  std::string s;
  std::string toString() const {
    return s;
  }
  friend std::ostream& operator<<(std::ostream& os, const Var &v){
    os << v.toString();
    return os;
  }

  Atom* clone() const  {
    return new Var(s);
  }
};

std::string vecToString(const std::vector<std::unique_ptr<Expr>>& v);
std::string vecToString(const std::vector<std::unique_ptr<Atom>>& v);
std::string vecToString(const std::vector<Bind>& binds);
std::string vecToString(const std::vector<Var>& vars);
std::string vecToString(const std::vector<AAlt>& aalts);
std::string vecToString(const std::vector<PAlt>& palts);

class UpdateFlag {
public:
  UpdateFlag(bool update):update(update){};
  bool update;
  std::string toString() const {
    return (update ? "@u" : "@n");
  }

  friend std::ostream& operator<<(std::ostream& os, const UpdateFlag &f){
    os << f.toString();
    return os;
  }
};

class LambdaForm {
public:
  LambdaForm(const std::vector<Var>& v1, const UpdateFlag& u, const std::vector<Var>& v2, Expr* e): v1(v1), u(u), v2(v2), e(e){};
  std::vector<Var> v1;
  UpdateFlag u;
  std::vector<Var> v2;
  Expr* e;

  std::string toString() const {
    return "{" + vecToString(v1) + "} " + u.toString() + " {" + vecToString(v2) + "} -> " + e->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const LambdaForm &l){
    os << l.toString();
    return os;
  }
};

class Bind {
public:
  Bind(Var v, LambdaForm lf): v(v), lf(lf){};
  Var v;
  LambdaForm lf;
  std::string toString() const {
    return v.toString() + " = " + lf.toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const Bind &b){
    os << b.v << " = " << b.lf;
    return os;
  }
};


class Prog {
public:
  Prog(const std::vector<Bind>& binds): binds(binds){};
  std::vector<Bind> binds;

  Prog& operator=(const Prog& other)
  {
    this->binds = other.binds;
    return *this;
  }
  std::string toString() const {
      std::stringstream ss;
      for (auto& e: binds) {
        ss << e << std::endl;
      }
      return ss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const Prog &p){
    os << p.toString();
    return os;
  }
};


class Dflt {
public:
  Dflt(){};
  virtual std::string toString() const = 0;

  friend std::ostream& operator<<(std::ostream& os, const Dflt &n){
    os << n.toString();
    return os;
  }
};

class Named: public Dflt {
public:
  Named(Var* v, Expr* e): v(v), e(e){};
  Var*v;
  Expr* e;

  std::string toString() const {
    return v->toString() + " -> " + e->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const Named &n){
    os << *(n.v) << " -> " << *(n.e);
    return os;
  }
};

class Unnamed: public Dflt {
public:
  Unnamed(Expr* e): e(e){};
  Expr* e;

  std::string toString() const {
    return "default -> " + e->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const Unnamed &u){
    os << u.toString();
    return os;
  }
};

class NoDflt: public Dflt {
public:
  NoDflt(){};

  std::string toString() const {
    return "";
  }

  friend std::ostream& operator<<(std::ostream& os, const NoDflt &u){
    os << u.toString();
    return os;
  }
};

class Literal: public Atom {
public:
  Literal(int i): i(i){};
  int i;

  std::string toString() const {
    std::stringstream ss;
    ss << i;
    return ss.str();
  }

  friend std::ostream& operator<<(std::ostream& os, const Literal &l){
    os << l.toString();
    return os;
  }

  Atom* clone() const {
    return new Literal(i);
  }

};

enum Prim {MUL, DIV, ADD, SUB};
inline std::string primToString(Prim p) {
  switch (p) {
  case MUL:
    return "*";
  case DIV:
    return "/";
  case ADD:
    return "+";
  case SUB:
    return "-";
  }
}

inline std::ostream& operator<<(std::ostream& os, const Prim p){
  return os << primToString(p);
}

class Constr {
public:
  Constr(std::string s): s(s){};
  std::string s;

  std::string toString() const {
    return s;
  }

  friend std::ostream& operator<<(std::ostream& os, const Constr &c){
    return os << c.toString();
  }
};

class LocalDef: public Expr {
public:
  LocalDef(const std::vector<Bind>& binds, Expr* e): binds(binds), e(e){};
  std::vector<Bind> binds;
  Expr* e;

  std::string toString() const {
    return "let " + vecToString(binds) + "in " + e->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const LocalDef &e) {
    return os << e.toString();
  }
};

class LocalRec: public Expr {
public:
  LocalRec(const std::vector<Bind>& binds, Expr* e): binds(binds), e(e){};
  std::vector<Bind> binds;
  Expr* e;

  std::string toString() const {
    return "letrec " + vecToString(binds) + "in " + e->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const LocalRec &l){
    return os << l.toString();
  }
};

class Case: public Expr {
public:
  Case(Expr* e, Alt* a): e(e), a(a){};
  Expr* e;
  Alt* a;

  std::string toString() const {
    return  "case " + e->toString() + " of " + a->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const Case &l) {
    return os << l.toString();
  }
};

class App: public Expr {
public:
  App(Var* v, const std::vector<std::unique_ptr<Atom>>& iargs): function_name(v){
    for (const auto& arg : iargs) {
      args.emplace_back(std::unique_ptr<Atom>(arg->clone()));
    }
  };
  Var* function_name;
  std::vector<std::unique_ptr<Atom>> args;

  std::string toString() const {
    return "app " + function_name->toString() + "{" + vecToString(args) + " }";
  }

  friend std::ostream& operator<<(std::ostream& os, const App &l){
    os << l.toString();
    return os;
  }
};

class SatConstr: public Expr {
public:
  SatConstr(Constr* c, const std::vector<std::unique_ptr<Atom>>& iargs): c(c){
    for (const auto& arg : iargs) {
      args.emplace_back(std::unique_ptr<Atom>(arg->clone()));
    }
  };
  Constr* c;
  std::vector<std::unique_ptr<Atom>> args;

  std::string toString() const {
    return c->toString() + "{" + vecToString(args) + "}";
  }

  friend std::ostream& operator<<(std::ostream& os, const SatConstr &l){
    return os << l.toString();
  }
};

class SatOp: public Expr {
public:
  SatOp(Prim prim, const std::vector<std::unique_ptr<Atom>>& iargs): prim(prim){
    for (const auto& arg : iargs) {
      args.emplace_back(std::unique_ptr<Atom>(arg->clone()));
    }
  };
  Prim prim;
  std::vector<std::unique_ptr<Atom>> args;

  std::string toString() const {
    return primToString(prim) + " { " + vecToString(args) + " }";
  }

  friend std::ostream& operator<<(std::ostream& os, const SatOp &l){
    os << l.toString();
    return os;
  }
};

class Lit: public Expr {
public:
  Lit(Literal* l): l(l){};
  Literal* l;

  std::string toString() const {
    return l->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const Lit &l){
    os << l.toString();
    return os;
  }
};


class AAlts: public Alt {
public:
  AAlts(const std::vector<AAlt>& v, Dflt* d): v(v), d(d){};
  std::vector<AAlt> v;
  Dflt* d;

  std::string toString() const {
    return "AAlts: " + vecToString(v) + d->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const AAlts &l){
    return os << l.toString();
  }
};

class PAlts: public Alt {
public:
  PAlts(const std::vector<PAlt>& v, Dflt* d): v(v), d(d){};
  std::vector<PAlt> v;
  Dflt* d;

  std::string toString() const {
    return "PAlts: " + vecToString(v) + d->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const PAlts &l){
    return os << l.toString();
  }
};

class Default: public Alt {
public:
  Default(Dflt* d): d(d){};
  Dflt* d;

  std::string toString() const {
    return d->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const Default &l){
    return os << l.toString();
  }
};

class AAlt {
public:
  AAlt(Constr* c, const std::vector<Var>& v, Expr* e): c(c), v(v), e(e){};
  Constr* c;
  std::vector<Var> v;
  Expr* e;

  std::string toString() const {
    return c->toString() + " {" + vecToString(v) + "} -> " +  e->toString();
  }

  friend std::ostream& operator<<(std::ostream& os, const AAlt &l){
    return os << l.toString();
  }
};

class PAlt {
public:
  PAlt(Literal* l, Expr* e): l(l), e(e){};
  Literal* l;
  Expr* e;

  std::string toString() const {
    return l->toString() + " {" + e->toString() + "}";
  }
  friend std::ostream& operator<<(std::ostream& os, const PAlt &l){
    return os << l.toString();
  }
};
#endif
