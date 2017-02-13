%{ /* -*- C++ -*- */
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <string>
#include "ast.hh"
#include "parser_driver.hh"
#include "parser.hh"

# undef yywrap
# define yywrap() 1
%}

%option noyywrap nounput batch debug noinput

%{
#define YY_USER_ACTION yylloc->columns(yyleng);
typedef yy::stg_parser::token token;
%}

%%
%{
// code run each time yylex is called
yylloc->step();
%}

[ \t]+             yylloc->step() ;
[\n]+              { yylloc->lines(yyleng); yylloc->step(); }
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
"of"               { return token::OF; }
"="                { return token::ASSIGN; }
[a-z][A-Za-z0-9_]* { yylval->sval = yytext; return token::VAR; }
[A-Z][A-Za-z0-9_]* { yylval->sval = yytext; return token::CONSTR; }
[0-9]*             { yylval->ival = atoi(yytext); return token::NUM; }
.                  { printf("Unknown character %c\n", *yytext); }
%%

void
parser_driver::scan_begin ()
{
    yy_flex_debug = trace_scanning;
    if (file.empty () || file == "-")
        yyin = stdin;
    else if (!(yyin = fopen (file.c_str (), "r")))
        {
            error ("cannot open " + file + ": " + strerror(errno));
            exit (EXIT_FAILURE);
        }
}

void
parser_driver::scan_end ()
{
    fclose (yyin);
}
