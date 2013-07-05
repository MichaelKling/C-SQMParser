/*
 * codegen.c -- ECO32 code generator
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
#include "codegen.h"
#define MAXREG 23
#define MINREG 7


typedef short int Reg;
typedef enum { loade, store, reference } Loadestore;

Reg registerStackPointer=7;

void assemblerProlog(FILE *outFile) {
  fprintf(outFile, "\t.import\tprinti\n");
  fprintf(outFile, "\t.import\tprintc\n");
  fprintf(outFile, "\t.import\treadi\n");
  fprintf(outFile, "\t.import\treadc\n");
  fprintf(outFile, "\t.import\texit\n");
  fprintf(outFile, "\t.import\ttime\n");
  fprintf(outFile, "\t.import\tclearAll\n");
  fprintf(outFile, "\t.import\tsetPixel\n");
  fprintf(outFile, "\t.import\tdrawLine\n");
  fprintf(outFile, "\t.import\tdrawCircle\n");
  fprintf(outFile, "\t.import\t_indexError\n");
  fprintf(outFile, "\n");
  fprintf(outFile, "\t.code\n");
  fprintf(outFile, "\t.align\t4\n");
}

/*===========================================================================================================================*/

void genCodeProc(Absyn *proc, Table *globalTable, FILE *outFile);                            /* ok - geisse */
void genCodeEmptyStm(FILE *outFile);                                                         /* ok - geisse */
void genCodeCompStm(Absyn *compStm,Table *globalTable, FILE *outFile);                       /* ok - geisse */
void genCodeAssignStm(Absyn *assignStm,Table *globalTable, FILE *outFile);                   /* ok - geisse */
void genCodeIfStm(Absyn *ifStm,Table *globalTable, FILE *outFile);                           /* ok - geisse */
void genCodeWhileStm(Absyn *whileStm,Table *globalTable, FILE *outFile);                     /* ok - geisse */
void genCodeCallStm(Absyn *callStm,Table *globalTable, FILE *outFile);                       /* ok - geisse */
void genCodeOpExpArith(Absyn *opExp,Table *globalTable, FILE *outFile);                      /* ok - geisse */
void genCodeOpExpBool(Absyn *opExp,Table *globalTable, FILE *outFile, int label);            /* ok - geisse */
void genCodeVarExp(Absyn *varExp,Table *globalTable, FILE *outFile, Loadestore flag);        /* ok - geisse */
void genCodeIntExp(Absyn *intExp,Table *globalTable, FILE *outFile);                         /* ok - geisse */
void genCodeSimpleVar(Absyn *simpleVar,Table *globalTable, FILE *outFile);                   /* ok - geisse  */
void genCodeArrayVar(Absyn *arrayVar,Table *globalTable, FILE *outFile, Loadestore flag);    /* TODO */
void genCodeStmList(Absyn *stmList,Table *globalTable, FILE *outFile);                       /* ok - geisse */
void genCodeStm(Absyn *stm,Table *globalTable, FILE *outFile);                               /* ok - geisse */
void genCodeDecList(Absyn *decList,Table *globalTable, FILE *outFile);                       /* ok - geisse */
void genCodeExp(Absyn *exp,Table *globalTable, FILE *outFile);                               /* ok - geisse */
void incRegisterStackPointer(void);
void decRegisterStackPointer(void);
Reg getRegisterStackPointer(void);
int genLabel(void);
int getOppositeCompare(int cmp);

/*===========================================================================================================================*/

void genCode(Absyn *program, Table *globalTable, FILE *outFile) {
  assemblerProlog(outFile);
  genCodeDecList(program,globalTable,outFile);
}


/*===========================================================================================================================*/

void genCodeProc(Absyn *proc, Table *globalTable, FILE *outFile) {
  Entry *procE = lookup(globalTable, proc->u.procDec.name);
  Table *table = procE->u.procEntry.localTable;
  int framesize;
  int framepointerpos;
  int returnregisterpos;

  if (proc->type != ABSYN_PROCDEC) {
    showAbsyn(proc);
    error("genCodeProc: Invalid Call.");
  }
/*
Prozedurdeklarationen
- Framegroesse berechnen
  ruft diese Prozedur andere Prozeduren?
  nein: Framegroesse = Platz für lokale Variablen + 4 Bytes für Framepointer
  ja:     Framegroesse = Platz für lokale Variablen + 8 Bytes für Framepointer und
          Returnadresse + Platz für ausgehende Argumente
*/
  if (procE->u.procEntry.size_out == -1) {
    framesize = REF_BYTE_SIZE + procE->u.procEntry.size_var;
    framepointerpos = 0;
  } else {
    framesize = procE->u.procEntry.size_var + procE->u.procEntry.size_out + REF_BYTE_SIZE + REF_BYTE_SIZE;
    framepointerpos = procE->u.procEntry.size_out + REF_BYTE_SIZE;
    returnregisterpos = 0 - REF_BYTE_SIZE - REF_BYTE_SIZE - procE->u.procEntry.size_var;
  }
/*
- Prozedur-Prolog ausgeben (->6.2) ( Label (Procname), Stackframe allokieren, fp
  und ggf Return-Adresse sichern, neuen Framepointer etablieren)
*/
  fprintf(outFile, "\n");
  fprintf(outFile, "\t.export\t%s\n",symToString(proc->u.procDec.name));
  fprintf(outFile, "%s:\n",symToString(proc->u.procDec.name));
  //allocate stack frame
  fprintf(outFile, "\tsub\t$29,$29,%d\t\t; allocate frame\n",framesize);
  //save old frame pointer
  fprintf(outFile, "\tstw\t$25,$29,%d\t\t; save old frame pointer\n",framepointerpos);
  //setup new frame pointer
  fprintf(outFile, "\tadd\t$25,$29,%d\t\t; setup new frame pointer\n",framesize);
  //save return register
  if (procE->u.procEntry.size_out != -1) {
    fprintf(outFile, "\tstw\t$31,$25,%d\t\t; save return register\n",returnregisterpos);
  }
/*
- Code für Prozedur-Koerper erzeugen
*/
  genCodeStmList(proc->u.procDec.body,table,outFile);
/*
- Prozedur-Epilog ausgeben (→6.2) (Framepointer und ggf Returnadresse
  wiederherstellen, Stackframe freigeben)
*/
  if (procE->u.procEntry.size_out != -1) {
    fprintf(outFile, "\tldw\t$31,$25,%d\t\t; restore return register\n",returnregisterpos);
  }
  fprintf(outFile, "\tldw\t$25,$29,%d\t\t; restore old frame pointer\n",framepointerpos);
  fprintf(outFile, "\tadd\t$29,$29,%d\t\t; release frame\n",framesize);
/*
- Prozedur verlassen
*/
  fprintf(outFile,"\tjr\t$31\t\t\t; return\n");

/*
    struct {
      Sym *name;
      struct absyn *params;
      struct absyn *decls;
      struct absyn *body;
    } procDec;
*/
}


/*===========================================================================================================================*/

void genCodeEmptyStm(FILE *outFile) {
  /* doing nothing */
//    struct {
//      int dummy;		/* empty struct not allowed in C */
//   } emptyStm;
}

/*===========================================================================================================================*/

void genCodeCompStm(Absyn *compStm,Table *globalTable, FILE *outFile) {
  if (compStm->type != ABSYN_COMPSTM) {
    showAbsyn(compStm);
    error("genCodeCompStm: Invalid Call.");
  }
  genCodeStmList(compStm->u.compStm.stms,globalTable,outFile);
//    struct {
//      struct absyn *stms;
//    } compStm;
}

/*===========================================================================================================================*/

void genCodeAssignStm(Absyn *assignStm,Table *globalTable, FILE *outFile) {
  int reg1,reg2;
  if (assignStm->type != ABSYN_ASSIGNSTM) {
    showAbsyn(assignStm);
    error("genCodeAssignStm: Invalid Call.");
  }
  incRegisterStackPointer();
  reg1 = getRegisterStackPointer();
  switch (assignStm->u.assignStm.var->type) {
    case ABSYN_SIMPLEVAR : genCodeSimpleVar(assignStm->u.assignStm.var,globalTable,outFile); break;
    case ABSYN_ARRAYVAR : genCodeArrayVar(assignStm->u.assignStm.var,globalTable,outFile,store); break;
    default: showAbsyn(assignStm);error("genCodeAssignStm unerwartetes Element gefunden.");
  }
  incRegisterStackPointer();
  reg2 = getRegisterStackPointer();
  genCodeExp(assignStm->u.assignStm.exp,globalTable,outFile);

  fprintf(outFile,"\tstw\t$%d,$%d,0\n",reg2,reg1);
 // genCodeVarExp(assignStm->u.assignStm.var,globalTable,outFile,store);

  decRegisterStackPointer();
  decRegisterStackPointer();
//    struct {
//      struct absyn *var;
//      struct absyn *exp;
//    } assignStm;
}

/*===========================================================================================================================*/

void genCodeIfStm(Absyn *ifStm,Table *globalTable, FILE *outFile) {
  int label1=genLabel();
  if (ifStm->type != ABSYN_IFSTM) {
    showAbsyn(ifStm);
    error("genCodeIfStm: Invalid Call.");
  }
  ifStm->u.ifStm.test->u.opExp.op = getOppositeCompare(ifStm->u.ifStm.test->u.opExp.op);
  genCodeOpExpBool(ifStm->u.ifStm.test,globalTable,outFile,label1);
  genCodeStmList(ifStm->u.ifStm.thenPart,globalTable,outFile);
  if (ifStm->u.ifStm.elsePart->type != ABSYN_EMPTYSTM) {
    int label2=genLabel();
    fprintf(outFile, "\tj\tL%d\n",label2);
    fprintf(outFile, "L%d:\n",label1);
    genCodeStmList(ifStm->u.ifStm.elsePart,globalTable,outFile);
    fprintf(outFile, "L%d:\n",label2);
  } else {
    fprintf(outFile, "L%d:\n",label1);
  }
//    struct {
//      struct absyn *test;
//      struct absyn *thenPart;
//      struct absyn *elsePart;
//    } ifStm;
}

/*===========================================================================================================================*/

void genCodeWhileStm(Absyn *whileStm,Table *globalTable, FILE *outFile) {
  int label1=genLabel();
  int label2=genLabel();
  if (whileStm->type != ABSYN_WHILESTM) {
    showAbsyn(whileStm);
    error("genCodeWhileStm: Invalid Call.");
  }
  whileStm->u.whileStm.test->u.opExp.op = getOppositeCompare(whileStm->u.whileStm.test->u.opExp.op);
  fprintf(outFile, "L%d:\n",label1);
  genCodeOpExpBool(whileStm->u.whileStm.test,globalTable,outFile, label2);
  genCodeStmList(whileStm->u.whileStm.body,globalTable,outFile);
  fprintf(outFile, "\tj\tL%d\n",label1);
  fprintf(outFile, "L%d:\n",label2);
  /* Register werden von der funktion selbst gemanaget: */
//    struct {
//      struct absyn *test;
//      struct absyn *body;
//    } whileStm;
}

/*===========================================================================================================================*/

void genCodeCallStm(Absyn *callStm,Table *globalTable, FILE *outFile) {
  Absyn *expList = callStm->u.callStm.args;
  ParamTypes *paramtypes;
  int counter = 0;

  if (callStm->type != ABSYN_CALLSTM) {
    showAbsyn(callStm);
    error("genCodeCallStm: Invalid Call.");
  }

  paramtypes = lookup(globalTable,callStm->u.callStm.name)->u.procEntry.paramTypes;
  while (!expList->u.expList.isEmpty) {
    incRegisterStackPointer();
    if (paramtypes->isRef) {
      genCodeVarExp(expList->u.expList.head,globalTable,outFile,reference);
    } else {
      genCodeExp(expList->u.expList.head,globalTable,outFile);
    }
    fprintf(outFile,"\tstw\t$%d,$29,%d\t\t; store arg #%d\n",getRegisterStackPointer(),counter*4,counter);
    decRegisterStackPointer();
    counter++;
    expList = expList->u.expList.tail;
    paramtypes = paramtypes->next;
  }
  //    struct {
  //      boolean isEmpty;
  //      struct absyn *head;
  //      struct absyn *tail;
  //    } expList;
  fprintf(outFile,"\tjal\t%s\n",symToString(callStm->u.callStm.name));
//    struct {
//      Sym *name;
//      struct absyn *args;
//    } callStm;
}

/*===========================================================================================================================*/

void genCodeExp(Absyn *exp,Table *globalTable, FILE *outFile) {
  switch(exp->type) {
    case ABSYN_OPEXP : genCodeOpExpArith(exp,globalTable,outFile); break;
    case ABSYN_VAREXP: genCodeVarExp(exp,globalTable,outFile,loade); break;
    case ABSYN_INTEXP: genCodeIntExp(exp,globalTable,outFile); break;
    default: showAbsyn(exp);error("genCodeOpExp unerwartetes Element gefunden.");
  }
}

/*===========================================================================================================================*/

void genCodeOpExpArith(Absyn *opExp,Table *globalTable, FILE *outFile) {

  if (opExp->type != ABSYN_OPEXP) {
    showAbsyn(opExp);
    error("genCodeOpExpArith: Invalid Call.");
  }
  //fprintf(outFile, "\t;arithmetische expression %d\n",opExp->u.opExp.op);
  genCodeExp(opExp->u.opExp.left,globalTable,outFile);
  Reg reg1=getRegisterStackPointer();
  incRegisterStackPointer();
  Reg reg2=getRegisterStackPointer();
  genCodeExp(opExp->u.opExp.right,globalTable,outFile);
  switch(opExp->u.opExp.op){
    case ABSYN_OP_ADD : fprintf(outFile, "\tadd\t$%d,$%d,$%d\n",reg1,reg1,reg2); break;
    case ABSYN_OP_SUB : fprintf(outFile, "\tsub\t$%d,$%d,$%d\n",reg1,reg1,reg2); break;
    case ABSYN_OP_MUL : fprintf(outFile, "\tmul\t$%d,$%d,$%d\n",reg1,reg1,reg2); break;
    case ABSYN_OP_DIV : fprintf(outFile, "\tdiv\t$%d,$%d,$%d\n",reg1,reg1,reg2); break;
//    struct {
//      int op;
//      struct absyn *left;
//      struct absyn *right;
//    } opExp;
    default: showAbsyn(opExp);error("genCodeOpExpArith unerwartetes Element gefunden.");
  }
  decRegisterStackPointer();
}

/*===========================================================================================================================*/

void genCodeOpExpBool(Absyn *opExp,Table *globalTable, FILE *outFile, int label) {

  if (opExp->type != ABSYN_OPEXP) {
    showAbsyn(opExp);
    error("genCodeOpExpBool: Invalid Call.");
  }

  //fprintf(outFile, "\t;boolean expression %d\n",opExp->u.opExp.op);
  incRegisterStackPointer();
  Reg reg1=getRegisterStackPointer();
  genCodeExp(opExp->u.opExp.left,globalTable,outFile);
  incRegisterStackPointer();
  Reg reg2=getRegisterStackPointer();
  genCodeExp(opExp->u.opExp.right,globalTable,outFile);
  switch(opExp->u.opExp.op){
    case ABSYN_OP_EQU : fprintf(outFile, "\tbeq\t$%d,$%d,L%d\n",reg1,reg2,label); break;
    case ABSYN_OP_NEQ : fprintf(outFile, "\tbne\t$%d,$%d,L%d\n",reg1,reg2,label); break;
    case ABSYN_OP_LST : fprintf(outFile, "\tblt\t$%d,$%d,L%d\n",reg1,reg2,label); break;
    case ABSYN_OP_LSE : fprintf(outFile, "\tble\t$%d,$%d,L%d\n",reg1,reg2,label); break;
    case ABSYN_OP_GRT : fprintf(outFile, "\tbgt\t$%d,$%d,L%d\n",reg1,reg2,label); break;
    case ABSYN_OP_GRE : fprintf(outFile, "\tbge\t$%d,$%d,L%d\n",reg1,reg2,label); break;
//    struct {
//      int op;
//      struct absyn *left;
//      struct absyn *right;
//    } opExp;
    default: showAbsyn(opExp);error("genCodeOpExpBool unerwartetes Element gefunden.");
  }
  decRegisterStackPointer();
  decRegisterStackPointer();
}

/*===========================================================================================================================*/

void genCodeVarExp(Absyn *varExp,Table *globalTable, FILE *outFile, Loadestore flag) {

  if (varExp->type != ABSYN_VAREXP) {
    showAbsyn(varExp);
    error("genCodeVarExp: Invalid Call.");
  }


  switch(varExp->u.varExp.var->type) {
      case ABSYN_SIMPLEVAR : genCodeSimpleVar(varExp->u.varExp.var,globalTable,outFile);
                           break;
      case ABSYN_ARRAYVAR  : genCodeArrayVar(varExp->u.varExp.var,globalTable,outFile,flag);
                           break;
      default : showAbsyn(varExp);error("genCodeVarExp unerwartetes Element gefunden.");
  }

  if(flag==store)
  {
    fprintf(outFile, "\tstw\t$%d,$%d,0\n",getRegisterStackPointer(),getRegisterStackPointer());
  }
  else if (flag==loade)
  {
    fprintf(outFile, "\tldw\t$%d,$%d,0\n",getRegisterStackPointer(),getRegisterStackPointer());
  }
  else if (flag==reference) {
  } else {
    error("Unknown flag in genCodeVarExp");
  }
//    struct {
//      struct absyn *var;
//    } varExp;
//struct {
//      Sym *name;
//    } simpleVar;
}

/*===========================================================================================================================*/

void genCodeIntExp(Absyn *intExp,Table *globalTable, FILE *outFile) {
  if (intExp->type != ABSYN_INTEXP) {
    showAbsyn(intExp);
    error("genCodeIntExp: Invalid Call.");
  }
  fprintf(outFile, "\tadd\t$%d,$0,%d\n",getRegisterStackPointer(),intExp->u.intExp.val);
//    struct {
//      int val;
//    } intExp;
}

/*===========================================================================================================================*/

void genCodeSimpleVar(Absyn *simpleVar,Table *globalTable, FILE *outFile) {
  int offset,reg;
  boolean isRef;
  Entry *varE;
  if (simpleVar->type != ABSYN_SIMPLEVAR) {
    showAbsyn(simpleVar);
    error("genCodeSimpleVar: Invalid Call.");
  }
  varE = lookup(globalTable,simpleVar->u.simpleVar.name);
  offset = varE->u.varEntry.loc;
  isRef = varE->u.varEntry.isRef;
  reg = getRegisterStackPointer();

  fprintf(outFile, "\tadd\t$%d,$25,%d\n",reg,offset);
  if (isRef) {
    fprintf(outFile, "\tldw\t$%d,$%d,0\n",reg,reg);
  }
//    struct {
//      Sym *name;
//    } simpleVar;
}

/*===========================================================================================================================*/


typedef struct typeStack {
  Type *element;
  struct typeStack* prev;
} TypeStack;

TypeStack *genCodeArrayVar_typeStack;
void pushTypeStack(TypeStack **stack,Type *element) {
  TypeStack *new = (TypeStack*)malloc(sizeof(TypeStack));
  new->element = element;
  new->prev = (*stack);
  (*stack) = new;
}

Type *popTypeStack(TypeStack **stack) {
  Type *element;
  TypeStack *old;
  if (stack == NULL) {
    error("popTypeStack: NULL Pointer Stack");
    return NULL;
  }
  element = (*stack)->element;
  old = (*stack);
  (*stack) = (*stack)->prev;
  free((void*)old);
  return element;
}

void genCodeArrayVarCalc(TypeStack *types,Absyn *var,int indices,Reg baseReg,Table *globalTable,FILE *outFile) {
  Type *type;
  if (var->type != ABSYN_ARRAYVAR) {
    return;
  }

  if (types == NULL) {
    error("genCodeArrayVarCalc: Invalid Type.");
  }
  type = popTypeStack(&types);
  if (type->kind == TYPE_KIND_PRIMITIVE) {
    error("genCodeArrayVarCalc: Invalid Type - Primitive.");
  }

  genCodeArrayVarCalc(types, var->u.arrayVar.var, indices * type->u.arrayType.size, baseReg,globalTable,outFile);

  // Hole Index und Speicher ihn auf eine Hilfsvariable (register)
  //pos = basePointer + arrayVar->u.arrayVar.index * (Dim(n-1)*Dim(n-2)*...*Dim(2)*Dim(1))  * primitiveSize;   /* index */
  incRegisterStackPointer();
  genCodeExp(var->u.arrayVar.index,globalTable,outFile);

  if (type->u.arrayType.size < 0) error("Array Index Calc error");
  incRegisterStackPointer();
  fprintf(outFile, "\tadd\t$%d,$0,%d\n",getRegisterStackPointer(),type->u.arrayType.size);
  // index pruefung
  fprintf(outFile, "\tbgeu\t$%d,$%d,_indexError\n",getRegisterStackPointer()-1,getRegisterStackPointer());
  fprintf(outFile, "\tmul\t$%d,$%d,%d\n",getRegisterStackPointer()-1,getRegisterStackPointer()-1,indices);

  decRegisterStackPointer();
  fprintf(outFile, "\tadd\t$%d,$%d,$%d\n",baseReg,baseReg,getRegisterStackPointer());
  decRegisterStackPointer();

  return;
}

void genCodeArrayVar(Absyn *arrayVar,Table *globalTable, FILE *outFile, Loadestore flag) {
  TypeStack *types=NULL;
  Absyn *var=NULL;
  Entry *entry=NULL;
  Type *type=NULL;
  int loc=0;

  if (arrayVar->type != ABSYN_ARRAYVAR) {
    showAbsyn(arrayVar);
    error("genCodeArrayVar: Invalid Call.");
  }

  /* kompletter abstieg um den symbol namen zu bekommen */
  var = arrayVar;
  while (var->type == ABSYN_ARRAYVAR) {
    var = var->u.arrayVar.var;
  }
  entry= lookup(globalTable,var->u.simpleVar.name);

  loc  = entry->u.varEntry.loc;
  type = entry->u.varEntry.type;

  //Typeliste "umdrehen"
  while (type->kind != TYPE_KIND_PRIMITIVE) {
    pushTypeStack(&types,type);
    type = type->u.arrayType.baseType;
  }

  //schreibt den offset auf den stack
  fprintf(outFile,"\tadd\t$%d,$25,%d\n",getRegisterStackPointer(),loc);

  if (entry->u.varEntry.isRef) {
    fprintf(outFile, "\tldw\t$%d,$%d,0\n",getRegisterStackPointer(),getRegisterStackPointer());
  }

  genCodeArrayVarCalc(types,arrayVar,INT_BYTE_SIZE,getRegisterStackPointer(),globalTable,outFile);
}

/*===========================================================================================================================*/

void genCodeStm(Absyn *stm,Table *globalTable, FILE *outFile) {
    switch(stm->type) {
      case ABSYN_EMPTYSTM : genCodeEmptyStm(outFile); break; /* nicht vergessen register aloc */
      case ABSYN_COMPSTM  : genCodeCompStm(stm,globalTable,outFile); break;
      case ABSYN_ASSIGNSTM: genCodeAssignStm(stm,globalTable,outFile); break;
      case ABSYN_IFSTM    : genCodeIfStm(stm,globalTable,outFile); break;
      case ABSYN_WHILESTM : genCodeWhileStm(stm,globalTable,outFile); break;
      case ABSYN_CALLSTM  : genCodeCallStm(stm,globalTable,outFile); break;
      default:  showAbsyn(stm);error("genCodeStm unerwartetes Element gefunden.");
    }
}

/*===========================================================================================================================*/

void genCodeStmList(Absyn *stmList,Table *globalTable, FILE *outFile) {
  if (stmList->type != ABSYN_STMLIST) {
    genCodeStm(stmList,globalTable,outFile);
    return;
  }
  if (stmList->type != ABSYN_STMLIST) {
    showAbsyn(stmList);
    error("genCodeStmList: Invalid Call.");
  }
  while (!stmList->u.stmList.isEmpty) {
    genCodeStm(stmList->u.stmList.head,globalTable,outFile);
    stmList = stmList->u.stmList.tail;
  }
}

/*===========================================================================================================================*/

void genCodeDecList(Absyn *decList,Table *globalTable, FILE *outFile) {
  if (decList->type != ABSYN_DECLIST) {
    showAbsyn(decList);
    error("genCodeDecList: Invalid Call.");
  }
  while (!decList->u.decList.isEmpty) {
    switch (decList->u.decList.head->type) {
      case ABSYN_PROCDEC : genCodeProc(decList->u.decList.head,globalTable,outFile);
                           break;
      case ABSYN_TYPEDEC : break;
      default : showAbsyn(decList);error("genCodeDecList unerwartetes Element gefunden.");
    }
    decList = decList->u.decList.tail;
  }
}

/*===========================================================================================================================*/

void incRegisterStackPointer(void)
{
    if(registerStackPointer+1>MAXREG)
    {
        error("Calculation is to complex (overflow)");
    }
    registerStackPointer++;
}

/*===========================================================================================================================*/

void decRegisterStackPointer(void)
{
    if(registerStackPointer-1<MINREG)
    {
        error("Stack (Register) underflow");
    }
    registerStackPointer--;
}

/*===========================================================================================================================*/

Reg getRegisterStackPointer(void)
{
    return registerStackPointer;
}

/*===========================================================================================================================*/

int genLabel(void) {
  static int lblNum = 0;
  return lblNum++;
}

/*===========================================================================================================================*/

int getOppositeCompare(int cmp) {
  switch(cmp){
    case ABSYN_OP_EQU : return ABSYN_OP_NEQ; break;
    case ABSYN_OP_NEQ : return ABSYN_OP_EQU; break;
    case ABSYN_OP_LST : return ABSYN_OP_GRE; break;
    case ABSYN_OP_LSE : return ABSYN_OP_GRT; break;
    case ABSYN_OP_GRT : return ABSYN_OP_LSE; break;
    case ABSYN_OP_GRE : return ABSYN_OP_LST; break;
//    struct {
//      int op;
//      struct absyn *left;
//      struct absyn *right;
//    } opExp;
    default: error("Invalid Boolean Operator - Can't build opposite.");
  }
  return -1;
}
