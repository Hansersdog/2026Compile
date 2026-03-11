%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "syntax.tab.h"
#include "tree.h"

extern int has_error;
extern int last_lex_error_line;

static int comment_start_line = 0;

static void lex_error(int line, const char* msg) {
  if (last_lex_error_line != line) {
    printf("Error type A at Line %d: %s.\n", line, msg);
    last_lex_error_line = line;
  }
  has_error = 1;
}

static void lex_error_mysterious(int line, const char* ch) {
  char buf[256];
  snprintf(buf, sizeof(buf), "Mysterious character \"%s\"", ch);
  lex_error(line, buf);
}

%}

%x COMMENT

DIGIT          [0-9]
HEX            [0-9a-fA-F]
OCT            [0-7]
ID             [A-Za-z_][A-Za-z0-9_]*
EXP            [eE][+-]?{DIGIT}+
FBASE1         {DIGIT}+"."{DIGIT}*
FBASE2         "."{DIGIT}+

%%

[ \t\r\f]+                   ;
\n                           ;

"//"[^\n]*                   ;
"/*"                         { comment_start_line = yylineno; BEGIN(COMMENT); }

<COMMENT>"*/"                { BEGIN(INITIAL); }
<COMMENT>\n                  ;
<COMMENT>.                   ;
<COMMENT><<EOF>>             { lex_error(comment_start_line, "Unclosed comment"); BEGIN(INITIAL); }

"struct"                     { yylval.node = node_token_none("STRUCT", yylineno); return STRUCT; }
"return"                     { yylval.node = node_token_none("RETURN", yylineno); return RETURN; }
"if"                         { yylval.node = node_token_none("IF", yylineno); return IF; }
"else"                       { yylval.node = node_token_none("ELSE", yylineno); return ELSE; }
"while"                      { yylval.node = node_token_none("WHILE", yylineno); return WHILE; }

"int"                        { yylval.node = node_token_str("TYPE", yylineno, yytext); return TYPE; }
"float"                      { yylval.node = node_token_str("TYPE", yylineno, yytext); return TYPE; }

";"                          { yylval.node = node_token_none("SEMI", yylineno); return SEMI; }
","                          { yylval.node = node_token_none("COMMA", yylineno); return COMMA; }
"="                          { yylval.node = node_token_none("ASSIGNOP", yylineno); return ASSIGNOP; }
">="|"<="|">"|"<"|"=="|"!="  { yylval.node = node_token_none("RELOP", yylineno); return RELOP; }
"+"                          { yylval.node = node_token_none("PLUS", yylineno); return PLUS; }
"-"                          { yylval.node = node_token_none("MINUS", yylineno); return MINUS; }
"*/"                         { yylval.node = node_token_none("BAD", yylineno); return BAD; }
"*"                          { yylval.node = node_token_none("STAR", yylineno); return STAR; }
"/"                          { yylval.node = node_token_none("DIV", yylineno); return DIV; }
"&&"                         { yylval.node = node_token_none("AND", yylineno); return AND; }
"||"                         { yylval.node = node_token_none("OR", yylineno); return OR; }
"."                          { yylval.node = node_token_none("DOT", yylineno); return DOT; }
"!"                          { yylval.node = node_token_none("NOT", yylineno); return NOT; }

"("                          { yylval.node = node_token_none("LP", yylineno); return LP; }
")"                          { yylval.node = node_token_none("RP", yylineno); return RP; }
"["                          { yylval.node = node_token_none("LB", yylineno); return LB; }
"]"                          { yylval.node = node_token_none("RB", yylineno); return RB; }
"{"                          { yylval.node = node_token_none("LC", yylineno); return LC; }
"}"                          { yylval.node = node_token_none("RC", yylineno); return RC; }

0[xX][A-Za-z0-9_]+           {
                               int ok = 1;
                               for (int i = 2; yytext[i]; i++) {
                                 if (!isxdigit((unsigned char)yytext[i])) { ok = 0; break; }
                               }
                               if (ok) {
                                 yylval.node = node_token_int("INT", yylineno, (int)strtol(yytext, NULL, 16));
                                 return INT;
                               }
                               char buf[256];
                               snprintf(buf, sizeof(buf), "Illegal hexadecimal number '%s'", yytext);
                               lex_error(yylineno, buf);
                               yylval.node = node_token_int("INT", yylineno, 0);
                               return INT;
                             }
0[xX]                        { lex_error(yylineno, "Illegal hexadecimal number '0x'"); yylval.node = node_token_int("INT", yylineno, 0); return INT; }
0{DIGIT}+                    {
                               int ok = 1;
                               for (int i = 1; yytext[i]; i++) {
                                 if (yytext[i] < '0' || yytext[i] > '7') { ok = 0; break; }
                               }
                               if (ok) {
                                 yylval.node = node_token_int("INT", yylineno, (int)strtol(yytext, NULL, 8));
                                 return INT;
                               }
                               char buf[256];
                               snprintf(buf, sizeof(buf), "Illegal octal number '%s'", yytext);
                               lex_error(yylineno, buf);
                               yylval.node = node_token_int("INT", yylineno, 0);
                               return INT;
                             }

({FBASE1}|{FBASE2}|{DIGIT}+)[eE][+-]?   {
                                         char buf[256];
                                         snprintf(buf, sizeof(buf), "Illegal floating point number \"%s\"", yytext);
                                         lex_error(yylineno, buf);
                                         yylval.node = node_token_float("FLOAT", yylineno, 0.0f);
                                         return FLOAT;
                                       }

{FBASE1}{EXP}                { yylval.node = node_token_float("FLOAT", yylineno, strtof(yytext, NULL)); return FLOAT; }
{FBASE2}{EXP}                { yylval.node = node_token_float("FLOAT", yylineno, strtof(yytext, NULL)); return FLOAT; }
{DIGIT}+{EXP}                { yylval.node = node_token_float("FLOAT", yylineno, strtof(yytext, NULL)); return FLOAT; }
{FBASE1}                     { yylval.node = node_token_float("FLOAT", yylineno, strtof(yytext, NULL)); return FLOAT; }
{FBASE2}                     { yylval.node = node_token_float("FLOAT", yylineno, strtof(yytext, NULL)); return FLOAT; }

0|[1-9]{DIGIT}*               { yylval.node = node_token_int("INT", yylineno, (int)strtol(yytext, NULL, 10)); return INT; }

{ID}                          { yylval.node = node_token_str("ID", yylineno, yytext); return ID; }

.                             { lex_error_mysterious(yylineno, yytext); }

%%
