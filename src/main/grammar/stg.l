%option noyywrap
%{
#include <cstdlib>
#include "ast.hh"
#include "parser.hh"
#define YY_DECL int yylex(yy::stg_parser::semantic_type *yylval, yy::stg_parser::location_type *yylloc)
// location includes current token
#define YY_USER_ACTION yylloc->columns (yyleng);

    typedef yy::stg_parser::token token;
%}

%%
%{
                         yylloc->step();
%}

[ \t]              ;
[\n]               { yylloc->lines(1); }
"default"          { return token::DEFAULT; }
"+"                { return token::ADD; }
"-"                { return token::SUB; }
"*"                { return token::MUL; }
"/"                { return token::DIV; }
"=="               { return token::EQL; }
"@u"               { return token::UPDATE; }
"@n"               { return token::NOUPDATE; }
"->"               { return token::ARROW; }
"}"                { return token::RBRACE; }
"{"                { return token::LBRACE; }
"let"              { return token::LET; }
"letrec"           { return token::LETREC; }
"case"             { return token::CASE; }
"in"               { return token::IN; }
";"                { return token::SEMI; }
"="                { return token::ASSIGN; }
[a-z][A-Za-z0-9_]* { return token::VAR; }
[A-Z][A-Za-z0-9_]* { return token::CONSTR; }
[0-9]*             { return token::NUM; }
.                  { printf("Unknown character %c\n", *yytext); }
%%
