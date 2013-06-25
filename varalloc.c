/*
 * varalloc.c -- variable allocation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "absyn.h"
#include "table.h"
#include "varalloc.h"

//Eintragen der vordefinierten Funktionen
void allocDefaultEntrys(Table *globalTable);
void computeOutgoingAreaSizes(Absyn *program,Table *globalTable);
int getMaxArgSize(Absyn *stmlist,Table *globalTable);
void showAlloc(Absyn *program,Table *globalTable);
void computeParamVars(Absyn *program,Table *globalTable);
int getSize(Type *type);

void allocVars(Absyn *program, Table *globalTable, boolean showVarAlloc) {
  /* compute access information for arguments of predefined procs */
  allocDefaultEntrys(globalTable);
  /* compute access information for arguments, parameters and local vars */
  computeParamVars(program,globalTable);
  /* compute outgoing area sizes */
  computeOutgoingAreaSizes(program,globalTable);
  /* show variable allocation if requested */
  if (showVarAlloc) {
    showAlloc(program,globalTable);
    exit(0);
  }
}

void computeOutgoingAreaSizes(Absyn *program,Table *globalTable) {
  Absyn *node,*dec;
  Entry *proc;
  node = program;
  while (!node->u.decList.isEmpty) {
    dec = node->u.decList.head;
    switch (dec->type) {
      case ABSYN_PROCDEC : proc = lookup(globalTable, dec->u.procDec.name);
                           proc->u.procEntry.size_out = getMaxArgSize(dec->u.procDec.body,globalTable);
                           break;
      default : ;
    }   
    node = node->u.decList.tail;
  }
  
}

int getMaxArgSizeStm(Absyn *stm,Table *globalTable,int max) {
  int buf = -1;
  switch (stm->type) {
      case ABSYN_COMPSTM: buf = getMaxArgSize(stm->u.compStm.stms,globalTable); 
                          if (buf > max) max = buf;
                          break;
      case ABSYN_IFSTM: buf = getMaxArgSize(stm->u.ifStm.thenPart,globalTable); 
                        if (buf > max) max = buf; 
                        buf = getMaxArgSize(stm->u.ifStm.elsePart,globalTable); 
                        if (buf > max) max = buf;  
                        break;
      case ABSYN_WHILESTM: buf = getMaxArgSize(stm->u.whileStm.body,globalTable); 
                           if (buf > max) max = buf;
                           break;
      case ABSYN_CALLSTM: { 
                            Entry *proc = lookup(globalTable, stm->u.callStm.name);
                            if (proc->u.procEntry.size_arg > max) max = proc->u.procEntry.size_arg;
                          }
                          break;
      default : ;
    }
    return max;
}

int getMaxArgSize(Absyn *stmlist,Table *globalTable) {
  Absyn *stm;
  int max = -1;
  if (stmlist->type != ABSYN_STMLIST) {
    return getMaxArgSizeStm(stmlist,globalTable,max);
  }
  while (!stmlist->u.stmList.isEmpty) {
    max = getMaxArgSizeStm(stmlist->u.stmList.head,globalTable,max);
    stmlist = stmlist->u.stmList.tail;
  }
  return max;
}

void showAlloc(Absyn *program,Table *globalTable) {
  Absyn *node,*dec;
  
  node = program;
  while (!node->u.decList.isEmpty) {
    dec = node->u.decList.head;
    switch (dec->type) {
      case ABSYN_PROCDEC : {
                             printf("\nVariable allocation for procedure '%s'\n",symToString(dec->u.procDec.name));
                             Entry *proc = lookup(globalTable, dec->u.procDec.name);
                             Absyn *param;
                             Entry *var;
                             int counter = 1;
                             Absyn *paramlist = dec->u.procDec.params;
                             while (!paramlist->u.decList.isEmpty) {
                               param = paramlist->u.decList.head;
                               var = lookup(proc->u.procEntry.localTable, param->u.parDec.name);
                               printf("arg %d: sp + %d\n",counter,var->u.varEntry.loc);
	                       paramlist = paramlist->u.decList.tail;
                               counter++;
	                     } 
                             printf("size of argument area = %d\n",proc->u.procEntry.size_arg);
                             paramlist = dec->u.procDec.params;
                             while (!paramlist->u.decList.isEmpty) {
                               param = paramlist->u.decList.head;
                               var = lookup(proc->u.procEntry.localTable, param->u.parDec.name);
                               printf("param '%s': fp + %d\n",symToString(param->u.parDec.name),var->u.varEntry.loc);
	                       paramlist = paramlist->u.decList.tail;
	                     } 
                             paramlist = dec->u.procDec.decls;
                             while (!paramlist->u.decList.isEmpty) {
                               param = paramlist->u.decList.head;
                               var = lookup(proc->u.procEntry.localTable, param->u.parDec.name);
                               printf("var '%s': fp - %d\n",symToString(param->u.parDec.name),-var->u.varEntry.loc);
	                       paramlist = paramlist->u.decList.tail;
	                     } 
                             printf("size of localvar area = %d\n",proc->u.procEntry.size_var);
                             printf("size of outgoing area = %d\n",proc->u.procEntry.size_out);
                           }
                           break;
      default : ;
    }   
    node = node->u.decList.tail;
  }
}

void computeParamVars(Absyn *program,Table *globalTable) {
  Absyn *node,*dec;
  
  node = program;
  while (!node->u.decList.isEmpty) {
    dec = node->u.decList.head;
    switch (dec->type) {
      case ABSYN_PROCDEC : {
                             Entry *proc = lookup(globalTable, dec->u.procDec.name);
                             Absyn *param;
                             Entry *var;
                             Absyn *paramlist = dec->u.procDec.params;
                             int size;
                             proc->u.procEntry.size_arg = 0;
                             while (!paramlist->u.decList.isEmpty) {
                               param = paramlist->u.decList.head;
                               var = lookup(proc->u.procEntry.localTable, param->u.parDec.name);
                               if (var->u.varEntry.isRef) {
                                 size = REF_BYTE_SIZE;
                               } else {
                                 size = INT_BYTE_SIZE;
                               }
                               var->u.varEntry.loc = proc->u.procEntry.size_arg;
                               proc->u.procEntry.size_arg += size;
	                       paramlist = paramlist->u.decList.tail;
	                     } 
                             proc->u.procEntry.size_var = 0;
                             paramlist = dec->u.procDec.decls;
                             while (!paramlist->u.decList.isEmpty) {
                               param = paramlist->u.decList.head;
                               var = lookup(proc->u.procEntry.localTable, param->u.parDec.name);
                               size = getSize(var->u.varEntry.type);
                               proc->u.procEntry.size_var += size;
                               var->u.varEntry.loc = -proc->u.procEntry.size_var;
	                       paramlist = paramlist->u.decList.tail;
	                     } 
                             proc->u.procEntry.size_out = 0;
                           }
                           break;
      default : ;
    }   
    node = node->u.decList.tail;
  }
}

void allocDefaultEntrys(Table *globalTable) {
  Entry *proc;
  
  //printi(i: int)
  proc = lookup(globalTable,newSym("printi"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = INT_BYTE_SIZE;
  //printc(i: int)
  proc = lookup(globalTable,newSym("printc"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = INT_BYTE_SIZE;
  //clearAll(i: int)
  proc = lookup(globalTable,newSym("clearAll"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = INT_BYTE_SIZE;

  //readi(ref i: int)
  proc = lookup(globalTable,newSym("readi"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = REF_BYTE_SIZE;
  //readc(ref i: int)
  proc = lookup(globalTable,newSym("readc"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = REF_BYTE_SIZE;
  //time(ref i: int)
  proc = lookup(globalTable,newSym("time"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = REF_BYTE_SIZE;

  //setPixel(x: int, y: int, color: int)
  proc = lookup(globalTable,newSym("setPixel"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = INT_BYTE_SIZE*3;
  
  //drawLine(x1: int, y1: int, x2: int, y2: int, color: int)
  proc = lookup(globalTable,newSym("drawLine"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = INT_BYTE_SIZE*5;

  //drawCircle(x0: int, y0: int, radius: int, color: int)
  proc = lookup(globalTable,newSym("drawCircle"));
  proc->u.procEntry.size_var = 0;
  proc->u.procEntry.size_out = 0;
  proc->u.procEntry.size_arg = INT_BYTE_SIZE*4;
}

int getSize(Type *type) {
  if (type->kind == TYPE_KIND_PRIMITIVE) {
    return INT_BYTE_SIZE;
  } else {
    return type->u.arrayType.size * getSize(type->u.arrayType.baseType);
  }
}
