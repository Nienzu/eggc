%{
#include <stdlib.h>
#include <string.h>
#include "calc3.h"
#include "test.tab.h"
void yyerror(char *);
%}
%%
id[A-Z][a-z]* {
                yylval.sIndex = strdup(yytext);
                return ID;
            }
0           {
                yylval.iValue = atoi(yytext);
                return NUM;
            }
[1-9][0-9]* {
                yylval.iValue = atoi(yytext);
                return NUM;
            }
[-()<>=+*/;{}.,!] {
                return *yytext;
             }
">="            {return GE;}
"<="            {return LE;}
"=="            {return EQ;}
"!="            {return NE;}
"return"        {return RETURN;}
"while"         {return WHILE;}
"if"            {return IF;}
"else"          {return ELSE;}
"print"         {return PRINT;}
"read"          {return READ;}
"int"           {yylval.sIndex = strdup(yytext);return INT;}
"char"          {yylval.sIndex = strdup(yytext);return CHAR;}
"&&"            {return AND;}
"||"            {return OR;}
"["             {return *yytext;}
"]"             {return *yytext;}


[ \t\n]+        ;       /* ignore whitespace */
.               {yyerror("Unknown character");}
%%
int yywrap(void) {
    return 1;
}
