#include <stdio.h>
#include <stdlib.h>
#include "calc3.h"
#include <string.h>
#include "test.tab.h"

/*SYMBOL TABLE*/
//write as assembly when main start.
int variable[30]; //[0-7]($s0-$s7) + [8-30](1-23)*4($sp)
static int lbl_IF=0;
static int lbl_WHILE=0;
int count=0;
int in_while = 0;
int sss[30]= {};
stack *now;
deflist *head;
deflist *next ,*current;
nodeType *start;
static int trindex=0;
char tr[5];
char iflist[7] = "IF";
static int srindex=0;
char sr[5];
int init=0;
char now_function[30];
//for function call
int call_count=0;


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
    int size_array;
    int id_type = -1;
    if(now == NULL) {
        now = malloc(sizeof(stack));
        now->down=now;
    }
    if (!p) return 0;
    switch(p->type) {
        case typeCon:
            now->con=p->con.value;
            now->stacktype=0;
            now = push(now);
            break;
        case typeId:
            now->id=p->id.i;
            if(p->id.is_array == -1) {
                now->stacktype=1;
            } else
                now->stacktype=3;
            now->is_array = p->id.is_array;
            now = push(now);
            break;
        case typeFun:
            if(init==0) {
                printf("\t.text\n");
                printf("\t.globl main\n");
                init++;
            }
            if(strcmp(p->funptr.name, "idMain")==0) {
                printf("main:\n");
            } else {
                printf("%s:\n",p->funptr.name);

            }
            strcpy(now_function,p->funptr.name);
            ex(p->funptr.op);
            if(strcmp(p->funptr.name, "idMain")==0) {
                printf("\tli $v0, 10\n");
                printf("\tsyscall\n");
            } else {
                printf("\tjr $ra\n");
            }
            break;
        case typeDef:

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
                    current = head;
                    while(strcmp(current->id_name,p->opr.op[0]->id.i) != 0 || strcmp(current->fun_name,now_function) != 0){
                      if(current->next->next != NULL)
                        current = current->next;
                      else{
                        current = current->next;
                        break;
                      }
                    }
                    id_type = current->type;
                    if(id_type == 1){
                      printf("\tli $v0, 5\n");
                      printf("\tsyscall\n");
                      printf("\tsw $v0, %s\n",p->opr.op[0]->id.i);
                    }
                    else if(id_type == 0){
                      printf("\tli $v0, 8\n");
                      printf("\tli $a1, 1\n");
                      printf("\rla $a0, %s\n",p->opr.op[0]->id.i);
                      printf("\tsyscall\n");
                    }
                    break;
                case PRINT:
                    current = head;
                    while(strcmp(current->id_name,p->opr.op[0]->id.i) != 0 || strcmp(current->fun_name,now_function) != 0){
                      if(current->next->next != NULL)
                        current = current->next;
                      else{
                        current = current->next;
                        break;
                      }
                    }
                    id_type = current->type;
                    if(id_type == 1){
                      printf("\tli $v0, 1\n");
                      printf("\tlw $a0, %s\n",p->opr.op[0]->id.i);
                      printf("\tsyscall\n");
                    }
                    else if(id_type == 0){
                      printf("\tli $v0, 4\n");
                      printf("\tla $a0, %s\n",p->opr.op[0]->id.i);
                      printf("\tsyscall\n");
                    }
                    break;
                case RETURN:
                    break;
                case '%':
                    printf("-----ARGUEMTN-----\n");
                    /*TODO: store $s0-$s7 and $t0-$t7 to stack.*/
                    /*TODO: put the argument into $a0-$a3*/
                    ex_call(p->opr.op[1]);
                    call_count=0;
                    printf("\tjal %s\n",p->opr.op[0]->id.i);
                    break;

                case '=':
                    ex(p->opr.op[1]);
                    //printf("\t %s, ", p->opr.op[0]->id.i);
                    if(p->opr.op[0]->id.is_array == -1) {
                        switch(now->down->stacktype) {
                            case 0:
                                printf("\tlw $s%d, %s\n",srindex, p->opr.op[0]->id.i);
                                printf("\tmove $s%d, %d\n", srindex, now->down->con);
                                printf("\tsw $s%d, %s\n", srindex, p->opr.op[0]->id.i);
                                break;
                            case 1:
                                printf("\tsw %s, %s\n", now->down->id, p->opr.op[0]->id.i);
                                break;
                            case 2:
                                printf("\tsw %s, %s\n", now->down->id, p->opr.op[0]->id.i);
                                break;
                            case 3: //array
                                printf("\tsw %d(%s), %s\n", now->down->is_array*4, now->down->id, p->opr.op[0]->id.i);
                            default:
                                printf("ERROR");
                                break;
                        }
                    } else {
                        switch(now->down->stacktype) {
                            case 0:
                                printf("\tlw $s%d, %d(%s)\n", srindex, p->opr.op[0]->id.is_array*4, p->opr.op[0]->id.i);
                                printf("\tmove $s%d, %d\n", srindex, now->down->con);
                                printf("\tsw $s%d, %d(%s)\n", srindex, p->opr.op[0]->id.is_array*4, p->opr.op[0]->id.i);
                                break;
                            case 1:
                                printf("\tsw %s, %d(%s)\n", now->down->id, p->opr.op[0]->id.is_array*4, p->opr.op[0]->id.i);
                                break;
                            case 2:
                                printf("\tsw %s, %d(%s)\n", now->down->id, p->opr.op[0]->id.is_array*4, p->opr.op[0]->id.i);
                                break;
                            case 3: //array
                                printf("\tsw %d(%s), %d(%s)\n", now->down->is_array*4, now->down->id, p->opr.op[0]->id.is_array*4, p->opr.op[0]->id.i);
                            default:
                                printf("ERROR");
                                break;
                        }
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
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tadd %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tadd %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tadd %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tadd %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tadd %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tadd %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tadd %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tadd %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tadd %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tsub %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsub %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsub %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsub %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsub %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsub %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsub %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsub %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsub %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tmul %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tmul %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tmul %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tmul %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tmul %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tmul %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tmul %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tmul %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tmul %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tdiv %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tdiv %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tdiv %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tdiv %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tdiv %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tdiv %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tdiv %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tdiv %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tdiv %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tslt %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tslt %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tslt %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tslt %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tslt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tslt %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tslt %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tslt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tslt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case '>':
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tsgt %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsgt %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsgt %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsgt %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsgt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsgt %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsgt %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsgt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsgt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case GE:
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tsge %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s", srindex, op2);
                                            printf("\tsge %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsge %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsge %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsge %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsge %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsge %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsge %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsge %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case LE:
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tsle %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsle %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsle %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsle %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsle %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsle %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsle %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsle %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsle %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case NE:
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tsne %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsne %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsne %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsne %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsne %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsne %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tsne %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tsne %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tsne %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case EQ:
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tseq %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tseq %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tseq %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tseq %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tseq %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tseq %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tseq %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tseq %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tseq %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case AND:
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tand %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tand %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tand %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tand %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tand %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tand %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tand %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tand %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tand %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                        case OR:
                            sprintf(tr,"$t%d", trindex);
                            count=2;
                            while(count!=0) {
                                switch(now->down->stacktype) {
                                    case 0:
                                        if(count == 2) {
                                            op1 = now->down->con;
                                            typeop1 = 0;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tor %s, %d, %d\n", tr, now->down->con, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tor %s, %d, $s%d\n", tr,now->down->con, srindex);
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tor %s, %d, $s%d\n", tr, now->down->con, srindex);
                                        }
                                        pop(now);
                                        break;
                                    case 1:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 1;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tor %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tor %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tor %s, $s%d, $s%d\n", tr,srindex-1, srindex);
                                            srindex--;
                                        }
                                        pop(now);
                                        break;
                                    case 3:
                                        if(count==2) {
                                            op2 = now->down->id;
                                            typeop1 = 3;
                                            size_array = now->down->is_array;
                                        } else if(count == 1 && typeop1 == 0) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            printf("\tor %s, $s%d, %d\n", tr, srindex, op1);
                                        } else if(count == 1 && typeop1 == 1) {
                                            printf("\tlw $s%d, %s\n", srindex, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %s\n", srindex, op2);
                                            printf("\tor %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        } else if(count == 1 && typeop1 == 3) {
                                            printf("\tlw $s%d, %d(%s)\n", srindex, now->down->is_array*4, now->down->id);
                                            srindex++;
                                            printf("\tlw $s%d, %d(%s)\n",srindex, size_array*4, op2);
                                            printf("\tor %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                            srindex--;
                                        }
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
                    }

            }
    }
    return 0;
}
int ex_def(nodeType *p)
{
    if(count==0) {
        printf("\t.data\n");
        count++;
    }
    if(head == NULL){
      head = malloc(sizeof(deflist));
      current = head;
    }
    if (!p) return 0;
    switch(p->type) {
        case typeFun:
            strcpy(now_function,p->funptr.name);
            ex_def(p->funptr.op);
            break;
        case typeDef:
            next = malloc(sizeof(deflist));
            next->type = -1;
            current->fun_name = strdup(now_function);
            current->id_name = strdup(p->def.name);
            current->is_array = p->def.is_array;
            current->next = next;
            if(strcmp(p->def.type, "int") == 0) {
                current->type = 1;
            } else {
                current->type = 0;
            }
            current = next;
            printf("%s:\t",p->def.name);
            if(p->def.is_array == -1) {
                if(strcmp(p->def.type, "int") == 0) {
                    printf(".word\t0\n");
                } else {
                    printf(".asciiz\t\"\"\n");
                }
            } else {
                if(strcmp(p->def.type, "int") == 0) {
                    printf(".space\t%d\n", p->def.is_array*4);
                } else {
                    printf(".space\t%d\n", p->def.is_array);
                }
            }
            break;
        case typeOpr:
            switch(p->opr.oper) {
                case ';':
                    ex_def(p->opr.op[0]);
                    ex_def(p->opr.op[1]);
                    break;
                default:
                    break;
            }
    }
    return 0;
}

int ex_call(nodeType *p)
{
    if (!p) return 0;
    switch(p->type) {
        case typeCon:
            printf("\tli $a%d, %d\n",call_count, p->con.value);
            call_count++;
            break;
        case typeId:
            printf("\tlw $s%d, %s\n",srindex, p->id.i);
            printf("\tmove $a%d, $s%d\n", call_count, srindex);
            call_count++;
            break;
        case typeOpr:
            switch(p->opr.oper) {
                case ',':
                    ex_call(p->opr.op[0]);
                    ex_call(p->opr.op[1]);
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    return 0;
}
