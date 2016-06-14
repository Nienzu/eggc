%{
#include "y.tab.h"
#include <stdio.h>
#define YYSTYPE char*
int lineno=0;
void yyerror(char *s){
	
	printf("ERROR syntax %d : %s\n",lineno, yytext);
}
int yywrap(void);

%}

APHA	[a-zA-Z]
NUMBER	[0-9]

%%
[ \t]+      { /* Ignore whitespace */ }
"int"	return INT;
"char"	return CHAR;
\n		lineno++;
0		{yylval=strdup(yytext);return INTEGER;}
[-()<>=+*/;,.{}] {
			  	return *yytext;
			  }
"["		return '[';
"]"		return ']';
"=="	return EQ;
"break"	return BREAK;
"!="	return NE;
"<="	return LE;
">="	return GE;
"&&"	return AND;
"||"	return OR;
[1-9]({NUMBER})*     {
				yylval = strdup(yytext); 
				return INTEGER;
			  }
"if"          { return IF; }
"while"       { return WHILE; }
"print"		  return PRINT;
"read"		  return READ;
	
"return"      { return RETURN; }
"id"[A-Z]({APHA}|{NUMBER})* {
				yylval = strdup(yytext);
				return IDENT;
			  }
"else"		  return ELSE;
%%
int yywrap(void){
	return 1;
}
