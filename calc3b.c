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
extern FILE *yyout;


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
                fprintf(yyout,"\t.text\n");
                fprintf(yyout,"\t.globl main\n");
                init++;
            }
            if(strcmp(p->funptr.name, "idMain")==0) {
                fprintf(yyout,"main:\n");
            } else {
                fprintf(yyout,"%s:\n",p->funptr.name);

            }
            strcpy(now_function,p->funptr.name);
            ex(p->funptr.op);
            if(strcmp(p->funptr.name, "idMain")==0) {
                fprintf(yyout,"\tli $v0, 10\n");
                fprintf(yyout,"\tsyscall\n");
            } else {
                fprintf(yyout,"\tjr $ra\n");
            }
            break;
        case typeDef:

            break;
        case typeOpr:
            switch(p->opr.oper) {
                case WHILE:
                    label_WHILE=lbl_WHILE;
                    fprintf(yyout,"while%d:\n",label_WHILE);
                    lbl_WHILE++;
                    ex(p->opr.op[0]);
                    fprintf(yyout,"\tbne %s, 1, while%d\n", tr,label_WHILE+1);
                    in_while = 1;
                    ex(p->opr.op[1]);
                    trindex=0;
                    ex(p->opr.op[0]);
                    fprintf(yyout,"\tbeq %s, 1, while%d\n", tr,label_WHILE);
                    fprintf(yyout,"while%d:\n", label_WHILE+1);
                    lbl_WHILE++;
                    in_while = 0;
                    break;
                case IF:
                    label_IF = lbl_IF;
                    sprintf(&tmp,"%d",label_IF);
                    iflist[strlen(iflist)] = tmp;
                    ex(p->opr.op[0]);
                    fprintf(yyout,"\tbne %s, 1, %s\n", tr, iflist);
                    ex(p->opr.op[1]);
                    label_IF = lbl_IF;
                    sprintf(&tmp,"%d",label_IF+1);
                    iflist[strlen(iflist) - 1] = tmp;
                    fprintf(yyout,"\tb %s\n", iflist);
                    iflist[strlen(iflist) - 1] = tmp-1;
                    fprintf(yyout,"%s:\n",  iflist);
                    lbl_IF++;
                    ex(p->opr.op[2]);
                    label_IF = lbl_IF;
                    sprintf(&tmp,"%d",label_IF);
                    iflist[strlen(iflist) - 1] = tmp;
                    fprintf(yyout,"%s:\n", iflist);
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
                        fprintf(yyout,"\tb while%d\n",lbl_WHILE);
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
                      fprintf(yyout,"\tli $v0, 5\n");
                      fprintf(yyout,"\tsyscall\n");
                      fprintf(yyout,"\tsw $v0, %s_%s\n",now_function,p->opr.op[0]->id.i);
                    }
                    else if(id_type == 0){
                      fprintf(yyout,"\tli $v0, 12\n");
                      fprintf(yyout,"\tsyscall\n");
                      fprintf(yyout,"\tsw $v0, %s_%s\n",now_function,p->opr.op[0]->id.i);
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
                      fprintf(yyout,"\tli $v0, 1\n");
                      fprintf(yyout,"\tlw $a0, %s_%s\n",now_function,p->opr.op[0]->id.i);
                      fprintf(yyout,"\tsyscall\n");
                    }
                    else if(id_type == 0){
                      fprintf(yyout,"\tli $v0, 11\n");
                      fprintf(yyout,"\tlw $a0, %s_%s\n",now_function,p->opr.op[0]->id.i);
                      fprintf(yyout,"\tsyscall\n");
                    }
                    break;
                case RETURN:
                    break;
                case '%':

                    //printf("-----ARGUEMTN-----\n");
                    /*TODO: store $s0-$s7 and $t0-$t7 to stack.*/
                    /*TODO: put the argument into $a0-$a3*/
                    //ex_call(p->opr.op[1]);
                    //call_count=0;

                    fprintf(yyout,"\tjal %s\n",p->opr.op[0]->id.i);
                    break;

                case '=':
                    ex(p->opr.op[1]);
                    //printf("\t %s, ", p->opr.op[0]->id.i);
                    if(p->opr.op[0]->id.is_array == -1) {
                        switch(now->down->stacktype) {
                            case 0:
                                fprintf(yyout,"\tli $t0, %d\n",now->down->con);
                                fprintf(yyout,"\tsw $t0, %s_%s\n", now_function,p->opr.op[0]->id.i);
                                break;
                            case 1:
                                if(now->down->id[0] == '$')
                                  fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                else
                                  fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                fprintf(yyout,"\tsw $s%d, %s_%s\n", srindex, now_function,p->opr.op[0]->id.i);
                                break;
                            case 2:
                                if(now->down->id[0] == '$')
                                  fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                else
                                  fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                fprintf(yyout,"\tsw $s%d, %s_%s\n", srindex, now_function,p->opr.op[0]->id.i);
                                break;
                            case 3: //array
                                fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,now->down->id);
                                fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex +1,now->down->is_array*4,srindex);
                                fprintf(yyout,"\tsw $s%d, %s_%s\n", srindex, now_function,p->opr.op[0]->id.i);
                            default:
                                printf("ERROR");
                                break;
                        }
                    } else {
                        switch(now->down->stacktype) {
                            case 0:
                                fprintf(yyout,"\tla $s%d, %s_%s\n", srindex,now_function, p->opr.op[0]->id.i);
                                fprintf(yyout,"\tli $t0, %d\n",now->down->con);
                                fprintf(yyout,"\tsw $t0, %d($s%d)\n", p->opr.op[0]->id.is_array*4,srindex);
                                break;
                            case 1:
                                fprintf(yyout,"\tla $s%d, %s_%s\n", srindex, now_function, p->opr.op[0]->id.i);
                                if(now->down->id[0] == '$')
                                  fprintf(yyout,"\tmove $s%d, %s_%s\n", srindex+1, now->down->id);
                                else
                                  fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,now->down->id);
                                fprintf(yyout,"\tsw $s%d, %d($s%d)\n", srindex+1, p->opr.op[0]->id.is_array*4,srindex);
                                break;
                            case 2:
                                fprintf(yyout,"\tla $s%d, %s_%s\n", srindex, now_function, p->opr.op[0]->id.i);
                                if(now->down->id[0] == '$')
                                  fprintf(yyout,"\tmove $s%d, %s_%s\n", srindex+1, now->down->id);
                                else
                                  fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,now->down->id);
                                fprintf(yyout,"\tsw $s%d, %d($s%d)\n", srindex+1, p->opr.op[0]->id.is_array*4,srindex);
                                break;
                            case 3: //array
                                fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,now->down->id);
                                fprintf(yyout,"\tla $s%d, %s_%s\n", srindex+1, now_function, p->opr.op[0]->id.i);
                                fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex +2,now->down->is_array*4,srindex);
                                fprintf(yyout,"\tsw $s%d, %d($s%d)\n", srindex+2, p->opr.op[0]->id.is_array*4,srindex+1);
                            default:
                                printf("ERROR");
                                break;
                        }
                    }
                    pop(now);
                    fprintf(yyout,"\n");
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tadd %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tadd %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tadd %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tadd %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tadd %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tadd %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tadd %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tadd %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tadd %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tsub %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tsub %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsub %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tsub %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tsub %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsub %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tsub %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tsub %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tsub %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tmul %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tmul %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tmul %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tmul %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tmul %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tmul %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tmul %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tmul %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tmul %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tdiv %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tdiv %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tdiv %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tdiv %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tdiv %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tdiv %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tdiv %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tdiv %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tdiv %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tslt %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tslt %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tslt %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tslt %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tslt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tslt %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tslt %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tslt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tslt %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tsgt %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tsgt %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsgt %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tsgt %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tsgt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsgt %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tsgt %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tsgt %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tsgt %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tsge %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tsge %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsge %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tsge %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tsge %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsge %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tsge %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tsge %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tsge %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tsle %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tsle %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsle %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tsle %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tsle %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsle %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tsle %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tsle %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tsle %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tsne %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tsne %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsne %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tsne %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tsne %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tsne %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tsne %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tsne %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tsne %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tseq %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tseq %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tseq %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tseq %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tseq %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tseq %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tseq %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tseq %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tseq %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tand %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tand %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tand %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tand %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tand %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tand %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tand %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tand %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tand %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          fprintf(yyout,"\tor %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          fprintf(yyout,"\tli $s%d, %d\n",srindex,now->down->con);
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex+1, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex+1, now_function,op2);
                                          fprintf(yyout,"\tor %s, $s%d, $s%d\n", tr, srindex, srindex+1);
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tor %s, $s%d, %d\n", tr, srindex+1,now->down->con);
                                      }
                                      pop(now);
                                      break;
                                  case 1:
                                      if(count==2) {
                                          op2 = now->down->id;
                                          typeop1 = 1;
                                      } else if(count == 1 && typeop1 == 0) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          fprintf(yyout,"\tor %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,op2);
                                          fprintf(yyout,"\tor %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function,now->down->id);
                                          srindex++;
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function,op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+1, size_array*4, srindex);
                                          fprintf(yyout,"\tor %s, $s%d, $s%d\n", tr,srindex-1, srindex+1);
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
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tor %s, $s%d, %d\n", tr, srindex, op1);
                                      } else if(count == 1 && typeop1 == 1) {
                                          if(now->down->id[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex, now->down->id);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, now->down->id);
                                          srindex++;
                                          if(op2[0] == '$')
                                              fprintf(yyout,"\tmove $s%d, %s\n", srindex,op2);
                                          else
                                              fprintf(yyout,"\tlw $s%d, %s_%s\n", srindex, now_function, op2);
                                          fprintf(yyout,"\tor %s, $s%d, $s%d\n", tr, srindex-1, srindex);
                                          srindex--;
                                      } else if(count == 1 && typeop1 == 3) {
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex, now_function, now->down->id);
                                          fprintf(yyout,"\tla $s%d, %s_%s\n",srindex+1, now_function, op2);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+2, now->down->is_array*4, srindex);
                                          fprintf(yyout,"\tlw $s%d, %d($s%d)\n",srindex+3, size_array*4, srindex+1);
                                          fprintf(yyout,"\tor %s, $s%d, $s%d\n", tr, srindex+2, srindex+3);
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
    argulist *a;
    if(count==0) {
        fprintf(yyout,"\t.data\n");
        count++;
    }
    if(head == NULL){
      head = malloc(sizeof(deflist));
      current = head;
    }
    if (!p) return 0;
    switch(p->type) {
        case typeFun:
            bzero(now_function,30);
            strcpy(now_function,p->funptr.name);
            a = p->funptr.argu;
            while(a->id!=NULL){
              fprintf(yyout,"%s_%s:\t",now_function,a->id);
              if(a->array == 0) {
                  if(strcmp(a->type, "int") == 0) {
                      fprintf(yyout,".word\t0\n");
                  } else {
                      fprintf(yyout,".asciiz\t\"\"\n");
                  }
              } else {
                  if(strcmp(a->type, "int") == 0) {
                      fprintf(yyout,".space\t%d\n", 32*4);
                  } else {
                      fprintf(yyout,".space\t%d\n", 32);
                  }
              }
              if(a->next != NULL)
                a = a->next;
              else
                break;
          }
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
            fprintf(yyout,"%s_%s:\t",now_function,p->def.name);
            if(p->def.is_array == -1) {
                if(strcmp(p->def.type, "int") == 0) {
                    fprintf(yyout,".word\t0\n");
                } else {
                    fprintf(yyout,".asciiz\t\"\"\n");
                }
            } else {
                if(strcmp(p->def.type, "int") == 0) {
                    fprintf(yyout,".space\t%d\n", p->def.is_array*4);
                } else {
                    fprintf(yyout,".space\t%d\n", p->def.is_array);
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
