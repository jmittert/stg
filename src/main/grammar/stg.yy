%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {stg_parser}
%define parse.assert
%code requires
{
#include <string>
class parser_driver;
}

%param { parser_driver& driver }
%locations
%initial-action {
    @$.begin.filename = @$.end.filename = &driver.file;
 }

/* Enable tracing and verbose errors*/
%define parse.trace
%define parse.error verbose

%code
{
#include "parser_driver.hh"
}

%{
#include <iostream>
using namespace std;
#include "ast.hh"
%}

%union {
  int ival;
  char* sval;
  Prog* p;
  Bind* b;
  LambdaForm* lf;
  std::vector<Bind>* bs;
  Expr* e;
  AAlt* aa;
  PAlt* pa;
  std::vector<AAlt>* aas;
  std::vector<PAlt>* pas;
  Dflt* d;
  Literal* l;
  Prim pr;
  Atom* at;
  std::vector<std::unique_ptr<Atom>>* ats;
  Var* v;
  std::vector<Var>* vs;
  Constr* c;
  UpdateFlag* u;
}


/* Tokens */
%token  <ival>          NUM
%token                  ADD SUB MUL DIV
%token                  LET LETREC CASE CASEP IN ASSIGN DEFAULT OF
%token                  LBRACE RBRACE ARROW
%token                  UPDATE NOUPDATE
%token  <sval>          VAR CONSTR

%type   <p>             program
%type   <b>             bind
%type   <bs>            binds
%type   <e>             expr
%type   <lf>            lambdaform
%type   <aa>            aalt
%type   <pa>            palt
%type   <d>             dflt
%type   <l>             lit
%type   <u>             update_flag
%type   <pr>             prim
%type   <at>            atom
%type   <ats>           atoms
%type   <v>             var
%type   <vs>            vars
%type   <c>             constr
%type   <pas>           palts
%type   <aas>           aalts
%%

program
: binds bind {{$1->push_back(*$2); driver.p = Prog(*$1); delete $1; delete $2;}}
;

bind
: var ASSIGN lambdaform {{$$=new Bind(*$1, *$3); delete $1; delete $3;}}
;

binds
:            {{$$ = new std::vector<Bind>();}}
| binds bind {{$1->push_back(*$2); delete $2; $$ = $1;}}
;

lambdaform
: LBRACE vars RBRACE update_flag LBRACE vars RBRACE ARROW expr {{$$ = new LambdaForm(*$2, *$4, *$6, shared_ptr<const Expr>($9)); delete $2; delete $4; delete $6;}}
;

expr
: LET    binds bind IN expr  {{$2->push_back(*$3); $$ = new LocalDef(*$2, shared_ptr<const Expr>($5)); delete $2; delete $3;}}
| LETREC binds bind IN expr  {{$2->push_back(*$3); $$ = new LocalRec(*$2, shared_ptr<const Expr>($5)); delete $2; delete $3;}}
| CASE expr OF LBRACE aalts dflt RBRACE    {{$$ = new Case(shared_ptr<const Expr>($2), std::shared_ptr<const Alt>(new AAlts(*$5, std::shared_ptr<const Dflt>($6)))); delete $5;}}
| CASEP expr OF LBRACE palts dflt RBRACE    {{$$ = new Case(shared_ptr<const Expr>($2), std::shared_ptr<const Alt>(new PAlts(*$5, std::shared_ptr<const Dflt>($6)))); delete $5;}}
| var LBRACE atoms RBRACE    {{$$ = new App(*$1, *$3); delete $1; delete $3;}}
| constr LBRACE atoms RBRACE {{$$ = new SatConstr(*$1, *$3); delete $1; delete $3;}}
| prim LBRACE atoms RBRACE   {{$$ = new SatOp($1, *$3); delete $3;}}
| lit                        {{$$ = new Lit(*$1); delete $1;}}
;

aalt
: constr LBRACE vars RBRACE ARROW expr {{ $$ = new AAlt(*$1, *$3, shared_ptr<const Expr>($6)); delete $1; delete $3;}}
;
palt
: lit ARROW expr {{ $$ = new PAlt(*$1, shared_ptr<const Expr>($3)); delete $1;}}
;

aalts
:            {{$$ = new std::vector<AAlt>();}}
| aalts aalt {{$1->push_back(*$2); delete $2; $$ = $1;}}

palts
:            {{$$ = new std::vector<PAlt>();}}
| palts palt {{$1->push_back(*$2); delete $2; $$ = $1;}}
;

dflt
:                {{$$ = new NoDflt(); }}
| var ARROW expr {{$$ = new Named(*$1, std::shared_ptr<Expr>($3)); delete $1;}}
| DEFAULT ARROW expr {{$$ = new Unnamed(std::shared_ptr<Expr>($3));}}
;

lit
: NUM      {{$$=new Literal($1);}}
;

update_flag
: UPDATE   {{$$ = new UpdateFlag(true);}}
| NOUPDATE {{$$ = new UpdateFlag(false);}}
;

prim
: ADD {{$$ = ADD;}}
| SUB {{$$ = SUB;}}
| MUL {{$$ = MUL;}}
| DIV {{$$ = DIV;}}
;

atom
: var {{$$ = $1;}}
| lit {{$$ = $1;}}
;

atoms
:            {{$$ = new std::vector<unique_ptr<Atom>>();}}
| atoms atom {{$1->emplace_back(unique_ptr<Atom>($2)); $$ = $1;}}
;

var
: VAR {{$$ = new Var($1);}}
;

vars
:          {{$$ = new std::vector<Var>();}}
| vars var {{$1->push_back(*$2); delete $2; $$ = $1;}}
;

constr
: CONSTR {{$$ = new Constr($1);}}
;

%%
void
yy::stg_parser::error (const location_type&l, const std::string& m)
{
    driver.error(l,m);
}
