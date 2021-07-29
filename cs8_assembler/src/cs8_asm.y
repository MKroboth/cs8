%code provides {
#define YY_DECL                             \
    int sslex ()

  // Declare the scanner.
  YY_DECL;
}
%code requires{
#include <src/Ast.h>
#include <string>
}
%{
#include <src/Ast.h>
#include <cstring>
#include <map>



extern int yylex();
extern void yyerror(AstRootNode** root, AstMemoryManager& ast, const char*);
#define YYDEBUG 1



std::map<std::string, std::string> defined_constants;
%}
%glr-parser
%define api.prefix ss
%define parse.trace
%define parse.error detailed
%parse-param { AstRootNode** root }
%parse-param { AstMemoryManager& ast }

%union
{
    int ival;
    char* sval;

    char* str;
    int number;
    AstRootNode* root_node;
    AstLineNode* line_node;
    AstParameterNode* parameter_node;
    std::list<AstParameterNode*>* parameter_nodes;
    std::list<AstLineNode*>* line_nodes;
    std::list<char*>* macro_args;
    Macro* macro;
};

%token <sval> TOK_UNKNOWN

%token <ival> TOK_NUMBER
%token <sval> TOK_STRING
%token <sval> TOK_IDENTIFIER
%token <sval> TOK_EMPTY_LINE

%token TOK_SPACE
%token TOK_COLON
%token TOK_PERCENT
%token TOK_COMMA
%token TOK_DOT
%token TOK_BACKSLASH

%token TOK_OPEN_PAR
%token TOK_CLOSE_PAR
%token TOK_OPEN_BRA
%token TOK_CLOSE_BRA
%token TOK_PLUS
%token TOK_MINUS
%token TOK_TIMES
%token TOK_DIVIDE

%token TOK_DEFINE
%token TOK_START_MACRO
%token TOK_END_MACRO


%left TOK_MINUS TOK_PLUS
%left TOK_TIMES TOK_DIV

%token TOK_NEWLINE

%type <line_nodes> lines nm_lines
%type <line_node> line nm_line instruction_0 instruction_n directive macro label instruction directive_n directive_0
%type <number> numeric
%type <number> substitution
%type <number> math_expr
%type <parameter_node> instruction_arg
%type <parameter_nodes> instruction_args
%type <parameter_node> directive_arg
%type <parameter_nodes> directive_args
%type <str> register
%type <str> name directive_name

%type <str> register_substitution
%type <str> macro_arg
%type <macro_args> macro_args
%type <parameter_node> replace_symbol symbol string_param
%type <macro> start_macro start_macro_0 start_macro_n



%%
file: lines  { for(auto* line: *$1) (*root)->add_line(line); }
| newline lines  { for(auto* line: *$2) (*root)->add_line(line); } ;

newline: TOK_NEWLINE | TOK_NEWLINE newline ;

name: TOK_IDENTIFIER { $$ = ast.copy_string($1); } ;

colon: TOK_COLON ;

label:
    name colon { $$ = ast.new_label($1); };

lines:
    line { $$ = ast.new_list<AstLineNode*>(); $$->push_back($1); }
    | line lines {   $$ = ast.new_list<AstLineNode*>(); $$->push_back($1); for(auto* line: *$2) $$->push_back(line); }
    ;

line:
    label
    | label newline
    | instruction newline
    | directive newline
    | macro newline
;

nm_lines:
    nm_line { $$ = ast.new_list<AstLineNode*>(); $$->push_back($1); }
    | nm_line nm_lines {   $$ = ast.new_list<AstLineNode*>(); $$->push_back($1); for(auto* line: *$2) $$->push_back(line); }
    ;


nm_line:
    instruction newline
    | directive newline
;

macro:
    start_macro newline nm_lines TOK_END_MACRO { $$ = ast.new_redact_line(); auto* macro = $1; for(auto* line: *$3) macro->add_line(line); (*root)->add_macro(std::move(*macro)); } ;

instruction:
    instruction_n
    | instruction_0
    ;

register:
    TOK_PERCENT TOK_IDENTIFIER { $$ = ast.copy_string($2); }
    ;

numeric:
    TOK_NUMBER { $$ = $1; }
    | substitution { $$ = $1; }
    | math_expr { $$ = $1; }
    ;

macro_arg:
    TOK_IDENTIFIER { $$ = ast.copy_string($1); }
    ;

macro_args:
    macro_arg { $$ = ast.new_list<char*>(); $$->push_back($1); }
    | macro_arg TOK_COMMA macro_args { $$ = ast.new_list<char*>(); $$->push_back($1);  for(auto* param: *$3) $$->push_back(param); }
    ;


start_macro_n: TOK_START_MACRO name macro_args {  $$ = new Macro($2); for(auto args: *$3) $$->add_arg(args); } ;
start_macro_0: TOK_START_MACRO name {  $$ = new Macro($2); } ;

start_macro: start_macro_n | start_macro_0 ;

substitution:
    TOK_OPEN_BRA name TOK_CLOSE_BRA { $$ = stoi(defined_constants.at($2)); }
;

register_substitution:
    TOK_PERCENT TOK_OPEN_BRA TOK_IDENTIFIER TOK_CLOSE_BRA { $$ = ast.copy_string(defined_constants.at($3).data()); }
;

replace_symbol:
    TOK_BACKSLASH name { $$ = ast.new_replace_symbol_parameter($2); } ;

symbol:
    name { $$ = ast.new_symbol_parameter($1); }

string_param: TOK_STRING { $$ = ast.new_string_parameter($1); } ;

math_expr:
  numeric TOK_PLUS numeric { $$ = $1 + $3; }
  | numeric TOK_MINUS numeric { $$ = $1 - $3; }
  | numeric TOK_TIMES numeric { $$ = $1 * $3; }
  | numeric TOK_DIVIDE numeric { $$ = $1 / $3; }
  ;

instruction_arg:
    register { $$ = ast.new_register_parameter($1); }
    | numeric { $$ = ast.new_number_parameter($1); }
    | register_substitution { $$ = ast.new_register_parameter($1); }
    | replace_symbol { $$ = $1; }
    | symbol { $$ = $1; }
    ;

instruction_args:
    instruction_arg { $$ = ast.new_list<AstParameterNode*>(); $$->push_back($1); }
    | instruction_arg TOK_COMMA instruction_args { $$ = ast.new_list<AstParameterNode*>(); $$->push_back($1);  for(auto* param: *$3) $$->push_back(param); }
    ;

directive_name: TOK_DOT TOK_IDENTIFIER { $$ = ast.copy_string($2); } ;

instruction_n: name instruction_args  { auto* result = ast.new_instruction($1); for(auto* param: *$2) result->add_parameter(param); $$ = result; } ;
instruction_0: name { $$ = ast.new_instruction($1); } ;

directive_arg:
     register { $$ = ast.new_register_parameter($1); }
        | numeric { $$ = ast.new_number_parameter($1); }
        | register_substitution { $$ = ast.new_register_parameter($1); }
        | replace_symbol { $$ = $1; }
        | symbol { $$ = $1; }
        | string_param { $$ = $1; }
        ;

directive_args:
    directive_arg { $$ = ast.new_list<AstParameterNode*>(); $$->push_back($1); }
    | directive_arg TOK_COMMA directive_args { $$ = ast.new_list<AstParameterNode*>(); $$->push_back($1);  for(auto* param: *$3) $$->push_back(param); }
    ;


directive_n: directive_name directive_args { auto* result = ast.new_directive($1); for(auto* param: *$2) result->add_parameter(param); $$ = result; } ;
directive_0: directive_name { $$ = ast.new_directive($1); } ;

directive: directive_n | directive_0 ;

%%
#include "scanner.h"
#include <cstdio>
#include <iostream>

void yyerror(AstRootNode** root, AstMemoryManager& ast, const char *s)
{
   printf("Error. %s\n", s);
   exit(-1);
}
AstRootNode parse(std::string filename, FILE* pt) {
    if(!pt)  {
    throw std::runtime_error("Bad Input.Noexistant file\n");

    }
    yyin = pt;

    AstRootNode root(filename);
    AstRootNode* rootptr = &root;
   {
    AstMemoryManager ast;

    do
    {
        yyparse(&rootptr, ast);

     }while (!feof(yyin));
    }



    return root;
}

