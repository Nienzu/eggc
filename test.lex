%{
#include <stdlib.h>
#include <string.h>
#include "calc3.h"
#include "test.tab.h"
void yyerror(char *);
%}
%%
id[A-Z][a-z]* {
                yylval.sIndex = strdup(yytext);ECHO;
                return ID;
            }
0           {
                yylval.iValue = atoi(yytext);ECHO;
                return NUM;
            }
[1-9][0-9]* {
                yylval.iValue = atoi(yytext);ECHO;
                return NUM;
            }
[-()<>=+*/;{}.,!] {ECHO;
                return *yytext;
             }
">="            {ECHO;return GE;}
"<="            {ECHO;return LE;}
"=="            {ECHO;return EQ;}
"!="            {ECHO;return NE;}
"return"        {ECHO;return RETURN;}
"while"         {ECHO;return WHILE;}
"if"            {ECHO;return IF;}
"else"          {ECHO;return ELSE;}
"print"         {ECHO;return PRINT;}
"read"          {ECHO;return READ;}
"int"           {ECHO;yylval.sIndex = strdup(yytext);return INT;}
"char"          {ECHO;yylval.sIndex = strdup(yytext);return CHAR;}
"&&"            {ECHO;return AND;}
"||"            {ECHO;return OR;}
"["             {ECHO;return *yytext;}
"]"             {ECHO;return *yytext;}


[ \t\n]+        ;       /* ignore whitespace */
.               {ECHO;yyerror("Unknown character");}
%%
int yywrap(void) {
    return 1;
}
