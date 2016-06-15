%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "calc3.h"
/* prototypes */
nodeType *opr(int oper, int nops, ...);
nodeType *id(char *i);
nodeType *con(int value);
nodeType *id_array(char *i,int value);
nodeType *d_id(char *i,char *y);
nodeType *d_id_array(char *i,char *y,int value);
nodeType *fun(char *i,char *y,argulist *d,nodeType *p);
argulist *argu(char *i,char *y,int value);
argulist *argu_append(char *i,char *y,argulist *d,int value);
void freeNode(nodeType *p);
int ex(nodeType *p);
int ex_def(nodeType *p);
int yylex(void);
void yyerror(char *s);
int sym[26];                    /* symbol table */
extern FILE *yyin;
%}
%union {
    int iValue;                 /* integer value */
    char *sIndex;                /* symbol table index */
    nodeType *nPtr;             /* node pointer */
    argulist *aPtr;             /* list pointer */
};
%token <iValue> NUM
%token <sIndex> ID
%token WHILE IF PRINT READ BREAK RETURN <sIndex> CHAR INT
%nonassoc ELSE
%left '+' '-' '*' '/' GE LE EQ NE '>' '<'
%left AND OR
%nonassoc UMINUS '!'
%type <sIndex> type
%type <aPtr> param_list
%type <nPtr> function stmt expr expr_list stmt_list
%%
program:
  function                {ex_def($1);ex($1); freeNode($1);exit(0); }
  ;
function:
    function stmt         { $$ = opr(';', 2, $1, $2); }
  | /* NULL */            { $$ = calloc(1,sizeof(nodeType));}
  ;
stmt:
    ';'                     { $$ = opr(';', 2, NULL, NULL); }
  | expr ';'                { $$ = $1; }
  | PRINT expr ';'          { $$ = opr(PRINT, 1, $2); }
  | READ expr ';'           { $$ = opr(READ, 1, $2);}
  | ID '=' expr ';'   { $$ = opr('=', 2, id($1), $3); }
  | ID '[' NUM ']' '=' expr ';' {$$ = opr('=', 2,id_array($1,$3),$6);}
  | BREAK ';'               { $$ = opr(BREAK, 0); }
  | RETURN expr ';'         { $$ = opr(RETURN, 1, $2); }
  | WHILE '(' expr ')' stmt { $$ = opr(WHILE, 2, $3, $5); }
  | IF '(' expr ')' stmt ELSE stmt
                            { $$ = opr(IF, 3, $3, $5, $7); }
  | '{' stmt_list '}'       { $$ = $2; }
  | type ID '(' param_list ')' '{' stmt_list '}' {$$ = fun($1, $2, $4, $7);}
  ;
stmt_list:
    stmt                  { $$ = $1; }
  | stmt_list stmt        { $$ = opr(';', 2, $1, $2); }
  ;
type:
  INT {$$ = $1;}
  | CHAR {$$ = $1;}
  ;
expr:
    NUM               { $$ = con($1); }
  | ID              {$$ = id($1);}
  | ID '[' NUM ']'  {$$ = id_array($1,$3);}
  | type ID             {$$ = d_id($1,$2);}
  | type ID '[' NUM ']' {$$ = d_id_array($1,$2,$4);}
  | '-' expr %prec UMINUS { $$ = opr(UMINUS, 1, $2); }
  | '!' expr              { $$ = opr('!', 1,$2);}
  | expr '+' expr         { $$ = opr('+', 2, $1, $3); }
  | expr '-' expr         { $$ = opr('-', 2, $1, $3); }
  | expr '*' expr         { $$ = opr('*', 2, $1, $3); }
  | expr '/' expr         { $$ = opr('/', 2, $1, $3); }
  | expr '<' expr         { $$ = opr('<', 2, $1, $3); }
  | expr '>' expr         { $$ = opr('>', 2, $1, $3); }
  | expr GE expr          { $$ = opr(GE, 2, $1, $3); }
  | expr LE expr          { $$ = opr(LE, 2, $1, $3); }
  | expr NE expr          { $$ = opr(NE, 2, $1, $3); }
  | expr EQ expr          { $$ = opr(EQ, 2, $1, $3); }
  | expr AND expr         { $$ = opr(AND, 2, $1, $3);}
  | expr OR expr          { $$ = opr(OR, 2, $1, $3);}
  | '(' expr ')'          { $$ = $2; }
  | ID '(' expr_list ')'  { $$ = opr('%', 2, id($1), $3);}
  ;
expr_list:
  expr                    { $$ = $1; }
  | expr_list ',' expr    { $$ = opr(',', 2, $1, $3); }
  ;
param_list:
  type ID                 { $$ = argu($1,$2,0); }
  | type ID '[' ']'       { $$ = argu($1,$2,1); }
  | type ID ',' param_list  { $$ = argu_append($1,$2,$4,0); }
  | type ID '[' ']' ',' param_list  { $$ = argu_append($1,$2,$6,1); }
  | { $$ = calloc(1,sizeof(argulist));}
  ;
%%
  #define SIZEOF_NODETYPE ((char *)&p->con - (char *)p)
  nodeType *con(int value) {
      nodeType *p;
      /* allocate node */
      if ((p = malloc(sizeof(nodeType))) == NULL)
          yyerror("out of memory");
      /* copy information */
      p->type = typeCon;
      p->con.value = value;
      return p;
  }
  nodeType *id(char *i) {
      nodeType *p;
      /* allocate node */
      if ((p = malloc(sizeof(nodeType))) == NULL)
          yyerror("out of memory");
      /* copy information */
      p->type = typeId;
      p->id.is_array = -1;
      if((p->id.i = malloc(sizeof(char) * strlen(i))) == NULL)
          yyerror("out of memory");
      strcpy(p->id.i,i);
      return p;
  }
  nodeType *fun(char *i,char *y,argulist *d,nodeType *p){
    nodeType *n;
    /* allocate node */
    if ((n = malloc(sizeof(nodeType))) == NULL)
        yyerror("out of memory");
    n->type = typeFun;
    n->funptr.type = strdup(i);
    n->funptr.name = strdup(y);
    n->funptr.argu = d;
    n->funptr.op = p;
    return n;
  }
  nodeType *id_array(char *i,int value){
    nodeType *p;
    /* allocate node */
    if ((p = malloc(sizeof(nodeType))) == NULL)
        yyerror("out of memory");
    /* copy information */
    p->type = typeId;
    p->id.is_array = value;
    if((p->id.i = malloc(sizeof(char) * strlen(i))) == NULL)
        yyerror("out of memory");
    strcpy(p->id.i,i);
    return p;
  }
  nodeType *d_id(char *i,char *j){
    nodeType *p;
    /* allocate node */
    if ((p = malloc(sizeof(nodeType))) == NULL)
        yyerror("out of memory");
    p->type = typeDef;
    p->def.type = strdup(i);
    p->def.name = strdup(j);
    p->def.is_array = -1;
    return p;
  }
  nodeType *d_id_array(char *i,char *y,int value){
    nodeType *p;
    /* allocate node */
    if ((p = malloc(sizeof(nodeType))) == NULL)
        yyerror("out of memory");
    p->type = typeDef;
    p->def.type = strdup(i);
    p->def.name = strdup(y);
    p->def.is_array = value;
    return p;
  }
  argulist *argu(char *i,char *y,int value){
    argulist *p;
    /* allocate node */
    if((p = malloc(sizeof(argulist))) == NULL)
      yyerror("out of memory");
    p->type = strdup(i);
    p->id= strdup(y);
    p->array = value;
    return p;
  }
  argulist *argu_append(char *i,char *y,argulist *d,int value){
    argulist *p;
    /* allocate node */
    if((p = malloc(sizeof(argulist))) == NULL)
      yyerror("out of memory");
    p->type = strdup(i);
    p->id= strdup(y);
    p->array = value;
    p->next = d;
    return p;
  }
  nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    nodeType *p;
    int i;
    /* allocate node, extending op array */
    if ((p = malloc(sizeof(nodeType) + (nops-1) * sizeof(nodeType *))) == NULL)
        yyerror("out of memory");
    /* copy information */
    p->type = typeOpr;
    p->opr.oper = oper;
    p->opr.nops = nops;
    va_start(ap, nops);
    for (i = 0; i < nops; i++)
        p->opr.op[i] = va_arg(ap, nodeType*);
    va_end(ap);
    return p;
}
void freeNode(nodeType *p) {
    int i;
    if (!p) return;
    if (p->type == 0) return;
    if (p->type == 2)
    if (p->type == typeOpr) {

        for (i = 0; i < p->opr.nops; i++)
            freeNode(p->opr.op[i]);
    }
    else if(p->type == typeFun){

      freeNode(p->funptr.op);
    }
    free (p);
}
void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}
int main(void) {
    yyin = fopen("input","r");
    yyparse();
    return 0;
}
