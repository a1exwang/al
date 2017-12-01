%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0"

%defines
%define parser_class_name { Parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { al }

%code requires
{
    #include <iostream>
    #include <string>
    #include <vector>
    #include <stdint.h>
    #include "ast.h"
    #include "compile_time.h"

    using namespace std;

    namespace al {
        class Lexer;
        class Token;
    }
}

%code top
{
    #include <iostream>
    #include "lex.h"
    #include "parser.tab.hpp"
    #include "location.hh"

    static al::Parser::symbol_type yylex(al::Lexer &lexer) {
        return lexer.lex();
    }

    using namespace al;
}

%code {
}

%lex-param { al::Lexer &lexer }
%parse-param { al::Lexer &lexer } { al::CompileTime &rt }
%locations
%define parse.trace
%define parse.error verbose

%token <std::shared_ptr<al::ast::StringLiteral>> STRING
%token <std::shared_ptr<al::ast::Symbol>> SYMBOL
%token <std::string> INT
%token QUOTE "'";
%token LEFTPAR "(";
%token RIGHTPAR ")";
%token SEMICOLON ";";
%token END 0 "end of file"

%type< std::shared_ptr<al::ast::Exp> > exp;
%type< std::shared_ptr<al::ast::ExpList> > exp_list;
%type< std::shared_ptr<al::ast::List> > list;

%start program

%%

program : exp { rt.setASTRoot($1); }

exp: list { $$ = $1; }
    | SYMBOL { $$ = $1; }
    | STRING { $$ = $1; }
    ;

list: LEFTPAR exp_list RIGHTPAR { $$ = std::make_shared<al::ast::List>($2); }

exp_list: { $$ = std::make_shared<al::ast::ExpList>(); }
    | exp exp_list { $$ = $2->prepend($1); }

%%
void al::Parser::error(const location &loc , const std::string &message) {
    cout << "Error: " << message << endl;
}
