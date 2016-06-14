typedef enum { typeCon, typeId, typeOpr, typeFun, typeDef} nodeEnum;
/* constants */
typedef struct {
    int value;                  /* value of constant */
} conNodeType;
/* identifiers */
typedef struct {
    char *i;                      /* subscript to sym array */
    int is_array;                 /* array index */
} idNodeType;
/* define */
typedef struct{
  char *type;                   /* type */
  char *name;                   /* name */
  int is_array;                 /* array size */
} defNodeType;
/* operators */
typedef struct {
    int oper;                   /* operator */
    int nops;                   /* number of operands */
    struct nodeTypeTag *op[1];  /* operands,extended at runtime*/
} oprNodeType;
/* function */
typedef struct{
  char *name;
  char *type;
  struct arguListTag *argu;
  struct nodeTypeTag *op;
}funNodeType;
typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node */
    union {
        conNodeType con;        /* constants */
        idNodeType id;          /* identifiers */
        oprNodeType opr;        /* operators */
        funNodeType funptr;     /* function */
        defNodeType def;        /* define */
    };
} nodeType;
/* argument list */
typedef struct arguListTag {
  char *type;
  char *id;
  int array;
  struct arguListTag *next;
} argulist;
extern int sym[26];