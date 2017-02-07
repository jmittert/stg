#ifndef AST_HH
#define AST_HH
#include <string>
#include <vector>
/*
 * Ast.hh: AST for STG
 */

class Expr;
class Alt;
class AAlt;
class PAlt;
class Atom {
public:
  Atom(){};
};

class Var: public Atom{
public:
  Var(std::string s): s(s){};
  std::string s;
};

class UpdateFlag {
public:
  UpdateFlag(bool update){};
  bool update;
};

class LambdaForm {
public:
  LambdaForm(std::vector<Var*>* v1, UpdateFlag* u, std::vector<Var*>* v2, Expr* e): v1(v1), u(u), v2(v2), e(e){};
  std::vector<Var*>* v1;
  UpdateFlag* u;
  std::vector<Var*>* v2;
  Expr* e;
};

class Bind {
public:
  Bind(Var* v, LambdaForm* lf): v(v), lf(lf){};
  Var* v;
  LambdaForm* lf;
};

class Prog {
public:
  Prog(std::vector<Bind*>* binds): binds(binds){};
  std::vector<Bind*>* binds;

  Prog& operator=(const Prog& other)
  {
    this->binds = other.binds;
    return *this;
  }
};


class Dflt {
public:
  Dflt(){};
};

class Named: public Dflt {
public:
  Named(Var* v, Expr* e): v(v), e(e){};
  Var*v;
  Expr* e;
};

class Unnamed: public Dflt {
public:
  Unnamed(Expr* e): e(e){};
  Expr* e;
};

class Literal: public Atom {
public:
  Literal(int i): i(i){};
  int i;
};

enum Prim {MUL, DIV, ADD, SUB, EQL};


class Constr {
public:
  Constr(std::string s): s(s){};
  std::string s;
};

class Expr {
public:
  Expr(){};
};

class LocalDef: public Expr {
public:
  LocalDef(std::vector<Bind*>* binds, Expr* e): binds(binds), e(e){};
  std::vector<Bind*>* binds;
  Expr* e;
};

class LocalRec: public Expr {
public:
  LocalRec(std::vector<Bind*>* binds, Expr* e): binds(binds), e(e){};
  std::vector<Bind*>* binds;
  Expr* e;
};

class Case: public Expr {
public:
  Case(Expr* e, Alt* a): e(e), a(a){};
  Expr* e;
  Alt* a;
};

class App: public Expr {
public:
  App(Var* v, std::vector<Atom*>* a): v(v), a(a){};
  Var* v;
  std::vector<Atom*>* a;
};

class SatConstr: public Expr {
public:
  SatConstr(Constr* c, std::vector<Atom*>* a ): c(c), a(a){};
  Constr* c;
  std::vector<Atom*>* a;
};

class SatOp: public Expr {
public:
  SatOp(Prim p, std::vector<Atom*>* a): p(p), a(a){};
  Prim p;
  std::vector<Atom*>* a;
};

class Lit: public Expr {
public:
  Lit(Literal* l): l(l){};
  Literal* l;
};

class Alt {
public:
  Alt(){};
};

class AAlts: public Alt {
public:
  AAlts(std::vector<AAlt*>* v, Dflt* d): v(v), d(d){};
  std::vector<AAlt*>* v;
  Dflt* d;
};

class PAlts: public Alt {
public:
  PAlts(std::vector<PAlt*>* v, Dflt* d): v(v), d(d){};
  std::vector<PAlt*>* v;
  Dflt* d;
};

class Default: public Alt {
public:
  Default(Dflt* d): d(d){};
  Dflt* d;
};

class AAlt {
public:
  AAlt(Constr* c, std::vector<Var*>* v, Expr* e): c(c), v(v), e(e){};
  Constr* c;
  std::vector<Var*>* v;
  Expr* e;
};

class PAlt {
public:
  PAlt(Literal* l, Expr* e): l(l), e(e){};
  Literal* l;
  Expr* e;
};
#endif
