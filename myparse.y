%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int yylex(void);
void yyerror(char *s);
FILE *out ;
%}



%token  INTEGER
%token TYPE
%token  IDENT
%token WHILE IF PRINT READ
%nonassoc IFX
%nonassoc ELSE
%token RETURN

%nonassoc ','
%left '>' '<'
%left '+' '-'
%left '*' '/'

%%

program:
	function program 
	|
	;

function:
	TYPE IDENT '(' parameter_list ')' '{' block '}'
	;

parameter_list:
	nonempty_parameter_list
	|
	;
nonempty_parameter_list:
	TYPE IDENT ',' parameter_list
	|
	TYPE IDENT
	;

block:
	statement block
	|
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
	TYPE IDENT '=' expression ';'
	|
	expression ';'
	|
	PRINT IDENT ';'
	|
	READ IDENT ';'
	|
	TYPE IDENT ';'
	|
	TYPE IDENT '[' expression ']' ';'
	|
	expression '=' expression ';'
	|
	IDENT '[' expression ']' '=' expression ';'
	;


expression:
	INTEGER
	|
	IDENT
	|
	IDENT '[' expression ']' 
	|
	IDENT '=' expression
	|
	expression '+' expression
	|
	expression '-' expression
	|
	expression '*' expression
	|
	expression '>' expression
	|
	IDENT '(' argument_list ')'
	|
	'(' expression ')'
	|
	expression '/' expression
	|
	;
%%
int main(){
	FILE *out=fopen("out.s", "wb");
	yyparse();
	return 0;
}
