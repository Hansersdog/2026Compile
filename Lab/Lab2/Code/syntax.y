%{
#include <stdio.h>
#include <string.h>
#include "tree.h"

extern int yylineno;
extern int has_error;
extern int last_syn_error_line;
extern Node* syntax_tree_root;
extern char* yytext;

int yylex(void);

static void syn_error(int line, const char* msg) {
  if (last_syn_error_line != line) {
    printf("Error type B at Line %d: %s.\n", line, msg);
    last_syn_error_line = line;
  }
  has_error = 1;
}

void yyerror(const char* s) {
  if (last_syn_error_line == yylineno) return;
  if (yytext && strcmp(yytext, ",") == 0) {
    syn_error(yylineno, "Missing \"]\"");
    return;
  }
  if (yytext && strcmp(yytext, "else") == 0) {
    syn_error(yylineno, "Missing \";\"");
    return;
  }
  syn_error(yylineno, "Syntax error");
}
%}

%union {
  Node* node;
}

%token <node> INT FLOAT ID TYPE
%token <node> STRUCT RETURN IF ELSE WHILE
%token <node> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT
%token <node> LP RP LB RB LC RC
%token <node> BAD

%type <node> Program ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def DecList Dec
%type <node> Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT UMINUS
%left DOT
%left LB RB
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program
  : ExtDefList
    { $$ = node_new("Program", $1 ? $1->line : yylineno, 0, 1, $1); syntax_tree_root = $$; }
  ;

ExtDefList
  : ExtDef ExtDefList
    {
      if ($1) $$ = node_new("ExtDefList", $1->line, 0, 2, $1, $2);
      else $$ = $2;
    }
  | 
    { $$ = NULL; }
  ;

ExtDef
  : Specifier ExtDecList SEMI
    { $$ = node_new("ExtDef", $1->line, 0, 3, $1, $2, $3); }
  | Specifier ExtDecList error
    { syn_error(yylineno, "Missing \";\""); yyerrok; $$ = node_new("ExtDef", $1->line, 0, 2, $1, $2); }
  | Specifier SEMI
    { $$ = node_new("ExtDef", $1->line, 0, 2, $1, $2); }
  | Specifier FunDec SEMI
    { $$ = node_new("ExtDef", $1->line, 0, 3, $1, $2, $3); }
  | Specifier FunDec CompSt
    { $$ = node_new("ExtDef", $1->line, 0, 3, $1, $2, $3); }
  | error SEMI
    { syn_error(yylineno, "Syntax error"); yyerrok; $$ = NULL; }
  ;

ExtDecList
  : VarDec
    { $$ = node_new("ExtDecList", $1->line, 0, 1, $1); }
  | VarDec COMMA ExtDecList
    { $$ = node_new("ExtDecList", $1->line, 0, 3, $1, $2, $3); }
  ;

Specifier
  : TYPE
    { $$ = node_new("Specifier", $1->line, 0, 1, $1); }
  | StructSpecifier
    { $$ = node_new("Specifier", $1->line, 0, 1, $1); }
  ;

StructSpecifier
  : STRUCT OptTag LC DefList RC
    { $$ = node_new("StructSpecifier", $1->line, 0, 5, $1, $2, $3, $4, $5); }
  | STRUCT Tag
    { $$ = node_new("StructSpecifier", $1->line, 0, 2, $1, $2); }
  ;

OptTag
  : ID
    { $$ = node_new("OptTag", $1->line, 0, 1, $1); }
  |
    { $$ = NULL; }
  ;

Tag
  : ID
    { $$ = node_new("Tag", $1->line, 0, 1, $1); }
  ;

VarDec
  : ID
    { $$ = node_new("VarDec", $1->line, 0, 1, $1); }
  | VarDec LB INT RB
    { $$ = node_new("VarDec", $1->line, 0, 4, $1, $2, $3, $4); }
  ;

FunDec
  : ID LP VarList RP
    { $$ = node_new("FunDec", $1->line, 0, 4, $1, $2, $3, $4); }
  | ID LP RP
    { $$ = node_new("FunDec", $1->line, 0, 3, $1, $2, $3); }
  ;

VarList
  : ParamDec COMMA VarList
    { $$ = node_new("VarList", $1->line, 0, 3, $1, $2, $3); }
  | ParamDec
    { $$ = node_new("VarList", $1->line, 0, 1, $1); }
  ;

ParamDec
  : Specifier VarDec
    { $$ = node_new("ParamDec", $1->line, 0, 2, $1, $2); }
  ;

CompSt
  : LC DefList StmtList RC
    { $$ = node_new("CompSt", $1->line, 0, 4, $1, $2, $3, $4); }
  ;

StmtList
  : Stmt StmtList
    {
      if ($1) $$ = node_new("StmtList", $1->line, 0, 2, $1, $2);
      else $$ = $2;
    }
  |
    { $$ = NULL; }
  ;

Stmt
  : Exp SEMI
    { $$ = node_new("Stmt", $1->line, 0, 2, $1, $2); }
  | Exp error
    { syn_error(yylineno, "Missing \";\""); yyerrok; $$ = node_new("Stmt", $1->line, 0, 1, $1); }
  | BAD
    { syn_error($1->line, "Syntax error"); $$ = NULL; }
  | CompSt
    { $$ = node_new("Stmt", $1->line, 0, 1, $1); }
  | RETURN Exp SEMI
    { $$ = node_new("Stmt", $1->line, 0, 3, $1, $2, $3); }
  | RETURN Exp error
    { syn_error(yylineno, "Missing \";\""); yyerrok; $$ = node_new("Stmt", $1->line, 0, 2, $1, $2); }
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
    { $$ = node_new("Stmt", $1->line, 0, 5, $1, $2, $3, $4, $5); }
  | IF LP Exp RP Stmt ELSE Stmt
    { $$ = node_new("Stmt", $1->line, 0, 7, $1, $2, $3, $4, $5, $6, $7); }
  | WHILE LP Exp RP Stmt
    { $$ = node_new("Stmt", $1->line, 0, 5, $1, $2, $3, $4, $5); }
  | error SEMI
    { syn_error(yylineno, "Syntax error"); yyerrok; $$ = NULL; }
  ;

DefList
  : Def DefList
    { $$ = node_new("DefList", $1->line, 0, 2, $1, $2); }
  | BAD DefList
    { syn_error($1->line, "Syntax error"); $$ = $2; }
  |
    { $$ = NULL; }
  ;

Def
  : Specifier DecList SEMI
    { $$ = node_new("Def", $1->line, 0, 3, $1, $2, $3); }
  | Specifier DecList error
    { syn_error(yylineno, "Missing \";\""); yyerrok; $$ = node_new("Def", $1->line, 0, 2, $1, $2); }
  ;

DecList
  : Dec
    { $$ = node_new("DecList", $1->line, 0, 1, $1); }
  | Dec COMMA DecList
    { $$ = node_new("DecList", $1->line, 0, 3, $1, $2, $3); }
  ;

Dec
  : VarDec
    { $$ = node_new("Dec", $1->line, 0, 1, $1); }
  | VarDec ASSIGNOP Exp
    { $$ = node_new("Dec", $1->line, 0, 3, $1, $2, $3); }
  ;

Exp
  : Exp ASSIGNOP Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp AND Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp OR Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp RELOP Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp PLUS Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp MINUS Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp STAR Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp DIV Exp
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | LP Exp RP
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | LP Exp error RP
    { syn_error(yylineno, "Missing \")\""); yyerrok; $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $4); }
  | MINUS Exp %prec UMINUS
    { $$ = node_new("Exp", $1->line, 0, 2, $1, $2); }
  | NOT Exp
    { $$ = node_new("Exp", $1->line, 0, 2, $1, $2); }
  | ID LP Args RP
    { $$ = node_new("Exp", $1->line, 0, 4, $1, $2, $3, $4); }
  | ID LP RP
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | Exp LB Exp RB
    { $$ = node_new("Exp", $1->line, 0, 4, $1, $2, $3, $4); }
  | Exp LB Exp error RB
    { syn_error(yylineno, "Missing \"]\""); yyerrok; $$ = node_new("Exp", $1->line, 0, 4, $1, $2, $3, $5); }
  | Exp DOT ID
    { $$ = node_new("Exp", $1->line, 0, 3, $1, $2, $3); }
  | ID
    { $$ = node_new("Exp", $1->line, 0, 1, $1); }
  | INT
    { $$ = node_new("Exp", $1->line, 0, 1, $1); }
  | FLOAT
    { $$ = node_new("Exp", $1->line, 0, 1, $1); }
  ;

Args
  : Exp COMMA Args
    { $$ = node_new("Args", $1->line, 0, 3, $1, $2, $3); }
  | Exp
    { $$ = node_new("Args", $1->line, 0, 1, $1); }
  ;

%%

#include "lex.yy.c"
