#include <stdio.h>
#include <stdlib.h>
#include "calc3.h"
#include <string.h>
#include "test.tab.h"

static int lbl_IF=0;
static int lbl_WHILE=0;
int count=0;
int in_while = 0;
int sss[30]= {};
stack *now;
static int trindex=0;
char tr[5];
char iflist[7] = "IF";
stack *push(stack *node)
{
    stack *new=malloc(sizeof(stack));
    new->down = node;
    new->top = NULL;
    node->top = new;
    return new;
}

void pop(stack *node)
{
    node->down = node->down->down;
    if(node->down->top != node)
      free(node->down->top);
    node->down->top = node;
}

int ex(nodeType *p)
{
    int op1;
    char *op2;
    char tmp;
    int typeop1;
    int label_IF;
    int label_WHILE;
    if(now == NULL) {
        now = malloc(sizeof(stack));
        now->down=now;
    }
    int i;
    if (!p) return 0;
    switch(p->type) {
        case typeCon:
            now->con=p->con.value;
            now->stacktype=0;
            now = push(now);
            break;
        case typeId:
            now->id=p->id.i;
            now->stacktype=1;
            now = push(now);
            break;
        case typeFun:
            ex(p->funptr.op);
            break;
        case typeOpr:
            switch(p->opr.oper) {
                case WHILE:
                    label_WHILE=lbl_WHILE;
                    printf("while%d:\n",label_WHILE);
                    lbl_WHILE++;
                    ex(p->opr.op[0]);
                    printf("\tbne %s, 1, while%d\n", tr,label_WHILE+1);
                    in_while = 1;
                    ex(p->opr.op[1]);
                    trindex=0;
                    ex(p->opr.op[0]);
                    printf("\tbeq %s, 1, while%d\n", tr,label_WHILE);
                    printf("while%d:\n", label_WHILE+1);
                    lbl_WHILE++;
                    in_while = 0;
                    break;
                case IF:
                    label_IF = lbl_IF;
                    sprintf(&tmp,"%d",label_IF);
                    iflist[strlen(iflist)] = tmp;
                    ex(p->opr.op[0]);
                    printf("\tbeq %s, 1, %s\n", tr, iflist);
                    ex(p->opr.op[1]);
                    label_IF = lbl_IF;
                    sprintf(&tmp,"%d",label_IF+1);
                    iflist[strlen(iflist) - 1] = tmp;
                    printf("\tb %s\n", iflist);
                    iflist[strlen(iflist) - 1] = tmp-1;
                    printf("%s:\n",  iflist);
                    lbl_IF++;
                    ex(p->opr.op[2]);
                    label_IF = lbl_IF;
                    sprintf(&tmp,"%d",label_IF);
                    iflist[strlen(iflist) - 1] = tmp;
                    printf("%s:\n", iflist);
                    lbl_IF++;
                    if(iflist[strlen(iflist) - 1] - iflist[strlen(iflist) - 2] == 1)
                      lbl_IF = iflist[strlen(iflist) - 2] - '0';
                    else if(iflist[strlen(iflist) - 1] - iflist[strlen(iflist) - 2] == 2)
                      lbl_IF = iflist[strlen(iflist) - 2] - '0' + 1;
                    iflist[strlen(iflist) - 1] = '\0';
                    trindex=0;
                    break;
                case BREAK:
                    if(in_while == 1)
                      printf("\tb while%d\n",lbl_WHILE);
                    break;
                case READ:
                    break;
                case PRINT:
                    printf("\tprint\n");
                    break;
                case RETURN:
                    break;
                case '=':
                    ex(p->opr.op[1]);
                    printf("\tmove %s, ", p->opr.op[0]->id.i);
                    switch(now->down->stacktype) {
                        case 0:
                            printf(" %d ", now->down->con);
                            break;
                        case 1:
                            printf(" %s ", now->down->id);
                            break;
                        case 2:
                            printf(" %s ", now->down->id);
                            break;
                        default:
                            printf("ERROR");
                    }
                    pop(now);
                    printf("\n");
                    trindex = 0;
                    break;
                case UMINUS:
                    ex(p->opr.op[0]);
                    printf("\tneg\n");
                    break;
                case '!':
                    ex(p->opr.op[0]);
                    printf("\tnot\n");
                    break;
                default:
                    ex(p->opr.op[0]);
                    ex(p->opr.op[1]);
                    switch(p->opr.oper) {
                        case '+':
                            sprintf(tr,"$t%d", trindex);
                            printf("\tadd %s, ",tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            now->id = strdup(tr);
                            now = push(now);

                            break;
                        case '-':
                            sprintf(tr,"$t%d", trindex);
                            printf("\tsub %s, ", tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            now->id = strdup(tr);
                            now = push(now);
                            break;
                        case '*':
                            sprintf(tr,"$t%d", trindex);
                            printf("\tmul %s, ",tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            now->id = strdup(tr);
                            now = push(now);
                            break;
                        case '/':
                            sprintf(tr,"$t%d", trindex);
                            printf("\tdiv %s, ",tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            now->id = strdup(tr);
                            now = push(now);
                            break;
                        case '<':
                        sprintf(tr,"$t%d", trindex);
                        printf("\tslt %s, ", tr);
                        count=2;
                        while(count!=0) {
                            switch(now->down->stacktype) {
                                case 0:
                                    if(count == 2) {
                                        op1 = now->down->con;
                                        typeop1 = 0;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %d, %d\n", now->down->con, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %d, %s\n",now->down->con, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                case 1:
                                    if(count==2) {
                                        op2 = now->down->id;
                                        typeop1 = 1;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %s, %d\n", now->down->id, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %s, %s\n", now->down->id, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                default:
                                    break;
                            }
                            count--;
                        }
                        now->stacktype=1;
                        sprintf(tr,"$t%d", trindex);
                        trindex++;
                        /*TODO: Temp register overflow detect*/
                        now->id = strdup(tr);
                        now = push(now);
                            break;
                        case '>':
                            sprintf(tr,"$t%d", trindex);
                            printf("\tsgt %s, ", tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        if(now->down->down!=NULL)
                                            pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        if(now->down->down!=NULL)
                                            pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            /*TODO: Temp register overflow detect*/
                            now->id = strdup(tr);
                            now = push(now);
                            break;
                        case GE:
                        sprintf(tr,"$t%d", trindex);
                        printf("\tsge %s, ", tr);
                        count=2;
                        while(count!=0) {
                            switch(now->down->stacktype) {
                                case 0:
                                    if(count == 2) {
                                        op1 = now->down->con;
                                        typeop1 = 0;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %d, %d\n", now->down->con, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %d, %s\n",now->down->con, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                case 1:
                                    if(count==2) {
                                        op2 = now->down->id;
                                        typeop1 = 1;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %s, %d\n", now->down->id, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %s, %s\n", now->down->id, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                default:
                                    break;
                            }
                            count--;
                        }
                        now->stacktype=1;
                        sprintf(tr,"$t%d", trindex);
                        trindex++;
                        /*TODO: Temp register overflow detect*/
                        now->id = strdup(tr);
                        now = push(now);
                            break;
                        case LE:
                        sprintf(tr,"$t%d", trindex);
                        printf("\tsle %s, ", tr);
                        count=2;
                        while(count!=0) {
                            switch(now->down->stacktype) {
                                case 0:
                                    if(count == 2) {
                                        op1 = now->down->con;
                                        typeop1 = 0;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %d, %d\n", now->down->con, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %d, %s\n",now->down->con, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                case 1:
                                    if(count==2) {
                                        op2 = now->down->id;
                                        typeop1 = 1;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %s, %d\n", now->down->id, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %s, %s\n", now->down->id, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                default:
                                    break;
                            }
                            count--;
                        }
                        now->stacktype=1;
                        sprintf(tr,"$t%d", trindex);
                        trindex++;
                        /*TODO: Temp register overflow detect*/
                        now->id = strdup(tr);
                        now = push(now);
                            break;
                        case NE:
                        sprintf(tr,"$t%d", trindex);
                        printf("\tsne %s, ", tr);
                        count=2;
                        while(count!=0) {
                            switch(now->down->stacktype) {
                                case 0:
                                    if(count == 2) {
                                        op1 = now->down->con;
                                        typeop1 = 0;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %d, %d\n", now->down->con, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %d, %s\n",now->down->con, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                case 1:
                                    if(count==2) {
                                        op2 = now->down->id;
                                        typeop1 = 1;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %s, %d\n", now->down->id, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %s, %s\n", now->down->id, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                default:
                                    break;
                            }
                            count--;
                        }
                        now->stacktype=1;
                        sprintf(tr,"$t%d", trindex);
                        trindex++;
                        /*TODO: Temp register overflow detect*/
                        now->id = strdup(tr);
                        now = push(now);
                            break;
                        case EQ:
                        sprintf(tr,"$t%d", trindex);
                        printf("\tseq %s, ", tr);
                        count=2;
                        while(count!=0) {
                            switch(now->down->stacktype) {
                                case 0:
                                    if(count == 2) {
                                        op1 = now->down->con;
                                        typeop1 = 0;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %d, %d\n", now->down->con, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %d, %s\n",now->down->con, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                case 1:
                                    if(count==2) {
                                        op2 = now->down->id;
                                        typeop1 = 1;
                                    } else if(count == 1 && typeop1 == 0)
                                        printf(" %s, %d\n", now->down->id, op1);
                                    else if(count == 1 && typeop1 == 1)
                                        printf(" %s, %s\n", now->down->id, op2);
                                    if(now->down->down!=NULL)
                                        pop(now);
                                    break;
                                default:
                                    break;
                            }
                            count--;
                        }
                        now->stacktype=1;
                        sprintf(tr,"$t%d", trindex);
                        trindex++;
                        /*TODO: Temp register overflow detect*/
                        now->id = strdup(tr);
                        now = push(now);
                            break;
                        case AND:
                            sprintf(tr,"$t%d", trindex);
                            printf("\tand %s, ", tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        if(now->down->down!=NULL)
                                            pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        if(now->down->down!=NULL)
                                            pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            /*TODO: Temp register overflow detect*/
                            now->id = strdup(tr);
                            now = push(now);
                            break;
                        case OR:
                            sprintf(tr,"$t%d", trindex);
                            printf("\tor %s, ", tr);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %d, %d\n", now->down->con, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %d, %s\n",now->down->con, op2);
                                        if(now->down->down!=NULL)
                                            pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0)
                                            printf(" %s, %d\n", now->down->id, op1);
                                        else if(count == 1 && typeop1 == 1)
                                            printf(" %s, %s\n", now->down->id, op2);
                                        if(now->down->down!=NULL)
                                            pop(now);
                                        break;
                                    default:
                                        break;
                                }
                                count--;
                            }
                            now->stacktype=1;
                            sprintf(tr,"$t%d", trindex);
                            trindex++;
                            /*TODO: Temp register overflow detect*/
                            now->id = strdup(tr);
                            now = push(now);
                            break;
                    }
            }
    }
    return 0;
}