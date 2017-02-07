%language "C++"
%defines
%locations

%define parser_class_name {stg_parser}

%{
#include <iostream>
using namespace std;
#include "include/ast.hh"
%}

%union {
    int ival;
    const char* sval;
    Prog* p;
    Bind* b;
    LambdaFrom* lf;
    std::vector<Bind*>* bs;
    Expr* e;
    Alt* a;
    std::vector<Alts*>* as;
    AAlt* aa;
    PAlt* pa;
    std::vector<AAlt*>* aas;
    std::vector<PAlt*>* pas;
    Dflt* d;
    Literal* l;
    Prim p;
    Atom* at;
    std::vector<Atom*>* ats;
    Var* v;
    std::vector<Var*>* vs;
    Constr* c;
    UpdateFlag* u;
}


/* Tokens */
%token  <ival>          NUM
%token                  ADD SUB MUL DIV EQL
%token                  LET LETREC CASE IN ASSIGN DEFAULT OF
%token                  SEMI LBRACE RBRACE ARROW
%token                  UPDATE NOUPDATE
%token  <sval>          VAR CONSTR

%type   <p>             program
%type   <b>             bind
%type   <bs>            binds
%type   <e>             expr
%type   <as>            alts
%type   <lf>            lambdaform
%type   <aa>            aalt
%type   <pa>            palt
%type   <d>             dflt
%type   <l>             lit
%type   <u>             update_flag
%type   <p>             prim
%type   <at>            atom
%type   <ats>           atoms
%type   <v>             var
%type   <vs>            vars
%type   <c>             constr
%type   <pas>           palts
%type   <aas>           aalts
%{
extern int yylex(yy::stg_parser::semantic_type **yylval,
                     yy::stg_parser::location_type* yylloc);
void myout(int val);
%}

%initial-action {
    @$.begin.filename = @$.end.filename = new std::string("stdin");
 }
%%

program
: binds bind {{$1.push_back($2); $$ = new Program($1);}}
;

bind
: var ASSIGN lambdaform SEMI {{$$=new Bind($1, $3);}}
;

binds
:          {{$$ = new std::vector<Bind*>();}}
| binds bind{{$1.push_back($2); $$ = $1;}}
;

lambdaform
: LBRACE vars RBRACE update_flag LBRACE vars RBRACE ARROW expr {{$$ = new LambdaFrom($2, $4, $6,$9);}}
;

expr
: LET binds bind IN expr     {{$3.push_back($2); $$ = new Letrec($2, $5);}}
| LETREC binds bind IN expr  {{$3.push_back($2); $$ = new Letrec($2, $5);}}
| CASE expr OF alts          {{$$ = new Case($2, $4);}}
| var LBRACE atoms RBRACE    {{$$ = new App($1, $3);}}
| constr LBRACE atoms RBRACE {{$$ = new SatConstr($1, $3);}}
| prim LBRACE atoms RBRACE   {{$$ = new SatOp($1, $3);}}
| lit                        {{$$ = new Lit($1); }}
;

alts
: dflt       {{$$ = new Default($1);}}
| aalts dflt {{$$ = new AAlts($1, $2);}}
| palts dflt {{$$ = new PAlts($1, $2);}}
;

aalts
:            {{$$ = new std::vector<AAlt*>();}}
| aalt aalts {{$1.push_back($2); $$ = $1;}}

aalt
: constr LBRACE vars RBRACE ARROW expr {{ $$ = new AAlt($1, $3, $6) }}
;
palt
: lit ARROW expr {{ $$ = new PAlt($1, $3); }}
;

palts
:            {{$$ = new std::vector<PAlt*>();}}
| palt palts {{$1.push_back($2); $$ = $1;}}
;

dflt
: var ARROW expr {{$$ = new Named($1, $3);}}
| DEFAULT ARROW expr {{$$ = new Unnamed($3);}}
;

lit
: NUM      {{$$=new Literal($1)}}
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
| EQL {{$$ = EQL;}}
;

atom
: var {{$$ = $1}}
| lit {{$$ = $1}}
;

atoms
:            {{$$ = new std::vector<Atom*>();}}
| atoms atom {{$1.push_back($2); $$ = $1;}}
;

var
: VAR {{$$ = new Var($1);}}
;
vars
:          {{$$ = new std::vector<Var*>();}}
| vars var {{$1.push_back($2); $$ = $1;}}
;
constr
: CONSTR {{$$ = new Constr($1);}}
;

%%
