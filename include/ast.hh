/*
 * Ast.hh: AST for STG
 */

class Prog {
public:
  Prog(std::vector<Bind*>* binds)binds(binds);
  vector*<Bind*> binds;
};

class Bind {
public:
  Bind(Var* v, LambdaForm* lf): v(v), lf(f){} ;
  Var* v;
  LambdaForm* lf;
};

class Expr {
};

class LocalDef: public Expr {
public:
  LocalDef(std::vector<Bind*>* binds, Expr* e): binds(binds), e(e);
  std::vector<Bind*>* binds;
  Expr* e;
};

class LocalRec: public Expr {
public:
  LocalRec(std::vector<Bind*>* binds, Expr* e): binds(binds), e(e);
  std::vector<Bind*>* binds;
  Expr* e;
};

class Case: public Expr {
public:
  Case(Expr* e, Alt* a): e(e), a(a);
  Expr* e;
  Alt* a;
};

class App: public Expr {
public:
  App(Var* v, std::vector<Atom*>* a): v(v), a(a);
  Var* v;
  std::vector<Atom*>* a;
};

class SatConstr: public Expr {
public:
  SatConstr(Constr* c, std::vec<Atom>a ): c(c), a(a);
  Constr* c;
  std::vector<Atom*>* a;
};

class SatOp: public Expr {
public:
  SatOp(Prim p, std::vector<Atom*>* a): p(p), a(a);
  Prim p;
  std::vector<Atom*>* a;
};

class Lit: public Expr {
public:
  Lit(Literal* l): l(l);
  Literal* l;
};

class Alt {
};

class AAlts: public Alt {
public:
  AAlt(std::vector<AAlt*>* v, Dflt d): v(v), d(d);
  std::vector<AAlt*>* v;
  Dflt* d;
};

class PAlts: public Alt {
public:
  AAlt(std::vector<PAlt*>* v, Dflt d): v(v), d(d);
  std::vector<PAlt*>* v;
  Dflt* d;
};

class Default: public Alt {
public:
  Default(Dflt* d): d(d);
  Dflt* d;
};

class AAlt {
  AAlt(Constr* c, std::vector<Var*>* v, Expr* e): c(c), v(v), e(e);
  Constr* c;
  std::vector<Var*>* v;
  Expr* e;
};

class PAlt {
  PAlt(Literal l, Expr e): l(l), e(e);
  Literal l;
  Expr e;
};

class Dflt {
};
class Named: public Dflt {
public:
  Named(Var& v, Expr& e): v(v), e(e);
  Var v;
  Expr e;
};

class Unnamed: public Dlft {
public:
  Unnamed(Expr& e): e(e);
  Expr e;
};

class Literal: Atom {
public:
  Literal(int i): i(i);
  int i;
}

class UpdateFlag {
public:
  UpdateFlag(bool update);
  bool update;
};

enum Prim {MUL, DIV, ADD, SUB};

class Atom {};

class Var: public Atom{
  Var(std::string s): s(s);
  std::string s;
};

class Constr {
  Constr(std::string s): s(s);
  std::string s;
};
