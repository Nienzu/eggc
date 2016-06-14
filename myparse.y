%{
#include<stdio.h>
#include<stdlib.h>
#define YYSTYPE char*
#include<string.h>

int yylex(void);
void yyerror(char *s);
FILE *out ;
%}



%token  INTEGER
%token INT CHAR
%token  IDENT
%token WHILE IF PRINT READ
%nonassoc IFX
%nonassoc ELSE
%token RETURN BREAK

%nonassoc ',' AND OR
%left '>' '<' GE LE NE EQ
%left '+' '-'
%left '*' '/'

%%

program:
	function program 
	|
	;

function:
	TYPE IDENT '(' parameter_list ')' '{' block '}'{printf("%s\n", $2);}
	;

parameter_list:
	nonempty_parameter_list {printf("\n");}
	| 
	;
nonempty_parameter_list:
	TYPE IDENT ',' parameter_list {printf(", %s %s",$1, $2);}
	|
	TYPE IDENT {printf("%s %s ",$1, $2);}
	;

block:
	statement block
	|
	;
TYPE:
	INT{$$="int";}
	|
	CHAR{$$="char";}
	;

argument_list:
	nonempty_argument_list
	|
	;
nonempty_argument_list:
	expression ',' nonempty_argument_list
	|
	expression
	;

statement:
	RETURN expression ';'
	|
	IF '(' expression ')' '{' block '}' ELSE '{' block '}'
	|
	WHILE '(' expression ')' '{' block '}'
	|
	PRINT IDENT ';' {printf("PRINT %s\n",$2);}
	|
	READ IDENT ';' {printf("READ %s\n",$2);}
	|
	TYPE IDENT ';' {printf("%s %s \n",$1, $2);}
	|
	TYPE IDENT '[' expression ']' ';'
	|
	expression '=' expression ';'{printf("= %s  \n", $1);}
	|
	BREAK ';'
	;
	

expression:
	INTEGER{printf(" %s ", $1);}
	|
	IDENT {printf(" %s ",$1);}
	|
	IDENT '[' expression ']' 
	|
	expression '+' expression {printf(" + ");}
	|
	expression '-' expression {printf(" - ");}
	|
	expression '*' expression {printf(" * ");}
	|
	expression '>' expression
	|
	expression EQ expression
	|
	expression NE expression
	|
	expression GE expression
	|
	expression LE expression
	|
	IDENT '(' argument_list ')'
	|
	expression AND expression
	|
	expression OR expression
	|
	'(' expression ')'
	|
	expression '/' expression {printf(" / ");}
	;
%%
int main(){
	FILE *out=fopen("out.s", "wb");
	yyparse();
	return 0;
}
