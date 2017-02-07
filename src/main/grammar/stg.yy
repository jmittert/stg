%language "C++"
%defines
%locations

%define parser_class_name {stg_parser}

%{
#include <iostream>
using namespace std;
#include "ast.hh"
%}

%parse-param {Prog &p_ret}
%union {
    int ival;
    char* sval;
    Prog* p;
    Bind* b;
    LambdaForm* lf;
    std::vector<Bind*>* bs;
    Expr* e;
    Alt* a;
    Alt* as;
    AAlt* aa;
    PAlt* pa;
    std::vector<AAlt*>* aas;
    std::vector<PAlt*>* pas;
    Dflt* d;
    Literal* l;
    Prim pr;
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
%token                  LBRACE RBRACE ARROW
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
%type   <pr>             prim
%type   <at>            atom
%type   <ats>           atoms
%type   <v>             var
%type   <vs>            vars
%type   <c>             constr
%type   <pas>           palts
%type   <aas>           aalts
%{
extern int yylex(yy::stg_parser::semantic_type *yylval,
                     yy::stg_parser::location_type* yylloc);
%}

%initial-action {
    @$.begin.filename = @$.end.filename = new std::string("stdin");
 }
%%

program
: binds bind {{$1->push_back($2); $$ = new Prog($1); p_ret = *$$;}}
;

bind
: var ASSIGN lambdaform {{$$=new Bind($1, $3);}}
;

binds
:          {{$$ = new std::vector<Bind*>();}}
| binds bind{{$1->push_back($2); $$ = $1;}}
;

lambdaform
: LBRACE vars RBRACE update_flag LBRACE vars RBRACE ARROW expr {{$$ = new LambdaForm($2, $4, $6,$9);}}
;

expr
: LET binds bind IN expr     {{$2->push_back($3); $$ = new LocalDef($2, $5);}}
| LETREC binds bind IN expr  {{$2->push_back($3); $$ = new LocalRec($2, $5);}}
| CASE expr OF alts          {{$$ = new Case($2, $4);}}
| var LBRACE atoms RBRACE    {{$$ = new App($1, $3);}}
| constr LBRACE atoms RBRACE {{$$ = new SatConstr($1, $3);}}
| prim LBRACE atoms RBRACE   {{$$ = new SatOp($1, $3);}}
| lit                        {{$$ = new Lit($1);}}
;

alts
: dflt       {{$$ = new Default($1);}}
| aalts dflt {{$$ = new AAlts($1, $2);}}
| palts dflt {{$$ = new PAlts($1, $2);}}
;


aalt
: constr LBRACE vars RBRACE ARROW expr {{ $$ = new AAlt($1, $3, $6);}}
;
palt
: lit ARROW expr {{ $$ = new PAlt($1, $3);}}
;

aalts
:            {{$$ = new std::vector<AAlt*>();}}
| aalts aalt {{$1->push_back($2); $$ = $1;}}

palts
:            {{$$ = new std::vector<PAlt*>();}}
| palts palt {{$1->push_back($2); $$ = $1;}}
;

dflt
: var ARROW expr {{$$ = new Named($1, $3);}}
| DEFAULT ARROW expr {{$$ = new Unnamed($3);}}
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
| EQL {{$$ = EQL;}}
;

atom
: var {{$$ = $1;}}
| lit {{$$ = $1;}}
;

atoms
:            {{$$ = new std::vector<Atom*>();}}
| atoms atom {{$1->push_back($2); $$ = $1;}}
;

var
: VAR {{$$ = new Var($1);}}
;

vars
:          {{$$ = new std::vector<Var*>();}}
| vars var {{$1->push_back($2); $$ = $1;}}
;

constr
: CONSTR {{$$ = new Constr($1);}}
;

%%

namespace yy {
    void stg_parser::error(location const &loc, const std::string& s) {
        std::cerr << "error at " << loc << ": " << s << std::endl;
    }
}

