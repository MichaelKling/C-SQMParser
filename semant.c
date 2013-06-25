/*
 * semant.c -- semantic analysis
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
#include "semant.h"

/*######################################################################################*/
/* Public: */
// Globaler boolean Typ (Grundtyp)
Entry *boolType = NULL;
//Globaler integer Typ (Grundtyp)
Entry *intType = NULL;
//Programmeinstiegspunkt Name (Zum suchen des main Symbols in der abstrakten Syntax)
Sym *symMain = NULL;

/*######################################################################################*/
/* Funktionen zur Fehlerausgabe */ 
void errorMsgRedeclarationEntry(Sym *sym,Entry *entry,int line);
void errorMsgRedeclarationParam(Sym *sym,int line);
void errorMsgRedeclaration(Sym *sym,char *type,int line);
void errorMsgUnknownElement(int line);

/*######################################################################################*/
/* Funktionen zur syntaktischen Analyse*/

//Hauptaufruf zur syntaktischen Analyse. Erwartet den Wurzelknoten der abstrakten Syntax
//und ein Flag ob er erstellte Symboltabellen ausgeben kann.
//Funktion bricht bei Fehlerhafter Semantik (aber korrekter Syntax) mit einer 
//Fehlermeldung ab.
//Bei ungueltiger Syntax ist der korrekte Programmablauf nicht gesichert.
//Zuruekgegeben wird die erstellte globale Symboltabelle
Table *check(Absyn *program, boolean showSymbolTables);

//Pruefen von Typdefinitionen
Type *checkType(Absyn *typeDec,Table *table);
//Pruefen von Proceduren
Entry *checkProc(Absyn *procDec,Table *table,boolean firstLook);
//Pruefen von Statemantlisten
void checkStmList(Absyn *stmlist,Table *table);
//Pruefen eines Statemants
void checkStm(Absyn *stm,Table *table);
//Pruefen einer Variable
Type *checkVar(Absyn *var,Table *table);
//Pruefen eines Ausdrucks
Type *checkExp(Absyn *exp,Table *table);
//Pruefen eines Tests (boolean exp)
void checkTest(Absyn *test,Table *table);
//Eintragen der vordefinierten Typen und Funktionen
void writeDefaultEntrys(Table *globalTable);

/*######################################################################################*/
/* Implementierung der Semantikanalyse */

Table *check(Absyn *program, boolean showSymbolTables) {
  Table *globalTable = NULL;
  Entry *mainProc = NULL;
  Absyn *node,*dec;
  Entry *prevDec,*currDec;

  // Erstellen der Basistypen 
  if (boolType == NULL) boolType = newTypeEntry(newPrimitiveType("bool"));
  if (intType == NULL) intType = newTypeEntry(newPrimitiveType("int"));
  if (symMain == NULL) symMain = newSym("main");

  //Erstellen der globalen Symboltabelle
  globalTable = newTable(NULL);

  // Dem Programmierer verfuegbare Basistypen in die Symboltabelle eintragen
  writeDefaultEntrys(globalTable);

  // Globale Symboltabelle erstellen + semantik Analyse
  node = program;
  while (!node->u.decList.isEmpty) {
    dec = node->u.decList.head;
    switch (dec->type) {
      case ABSYN_TYPEDEC : prevDec = lookup(globalTable, dec->u.typeDec.name);
                           if (prevDec != NULL) {
                                errorMsgRedeclarationEntry(dec->u.typeDec.name,prevDec,dec->line);
			   }
                           enter(globalTable, dec->u.typeDec.name, newTypeEntry(checkType(dec->u.typeDec.ty,globalTable)) );
                           break;
      case ABSYN_PROCDEC : prevDec = lookup(globalTable, dec->u.procDec.name);
                           if (prevDec != NULL) {
                                errorMsgRedeclarationEntry(dec->u.typeDec.name,prevDec,dec->line);
			   }
                           currDec = checkProc(dec,globalTable,TRUE);
                           if (symToStamp(symMain) == symToStamp(dec->u.procDec.name)) { 
                             mainProc = currDec; 
                           }
                           enter(globalTable, dec->u.procDec.name, currDec );
                           break;
      default : errorMsgUnknownElement(dec->line);
    }   
    node = node->u.decList.tail;
  }
  
  // zweiter durchlauf durch die procedur ruempfe/lokale tabellen + semantik
  node = program;
  while (!node->u.decList.isEmpty) {
    dec = node->u.decList.head;
    switch (dec->type) {
      case ABSYN_TYPEDEC : break;
      case ABSYN_PROCDEC : currDec = checkProc(dec,globalTable,FALSE);

                           /* Ausgeben der Tabelle */
                           if (showSymbolTables) {
                             printf("symbol table at end of procedure '%s':\n",symToString(dec->u.procDec.name));
                             showTable(currDec->u.procEntry.localTable);
                           }
                           break;
      default : errorMsgUnknownElement(dec->line);
    }   
    node = node->u.decList.tail;
  }


  /* check if "main()" is present */
  if (mainProc == NULL) { error("procedure 'main' is missing or not a procedure"); }  
  /* pruefen ob main eine procedure ist */
  if (mainProc->kind != ENTRY_KIND_PROC) { error("'main' is not a procedure"); } 
  /* pruefen ob main parameter erwartet */    
  if (!mainProc->u.procEntry.paramTypes->isEmpty) { error("procedure 'main' must not have any parameters"); }

  /* return global symbol table */
  return globalTable;
}

/*--------------------------------------------------------------------------------------*/

Type *checkType(Absyn *typeDec,Table *table) {
  Entry *entry;
  Type *typ;
  switch (typeDec->type) {
    case ABSYN_NAMETY: entry = lookup(table, typeDec->u.nameTy.name);
		       if (entry == NULL) {
                         error("undefined type '%s' in line %d",symToString(typeDec->u.nameTy.name),typeDec->line);
                       }
                       if (entry->kind != ENTRY_KIND_TYPE) {
                         error("'%s' is not a type in line %d",symToString(typeDec->u.nameTy.name),typeDec->line);
                       }	
                       return entry->u.typeEntry.type;
                       break;
    case ABSYN_ARRAYTY:	if (typeDec->u.arrayTy.size < 0) error("illegal index size in line %d",typeDec->line);
                        return newArrayType(typeDec->u.arrayTy.size,checkType(typeDec->u.arrayTy.ty,table));
                        break;  
    default : errorMsgUnknownElement(typeDec->line);
  }   	
  error("program intern error");
  return NULL;
}

/*--------------------------------------------------------------------------------------*/

//Hilfsfunktion um Parameterliste rekursive aufzubauen (Reihenfolge umdrehen)
ParamTypes *checkProcHelperBuildParam(Absyn *paramlist,Table *table) {
  Absyn *param;
  Entry *prevDec;
  Type *type;
  ParamTypes *pt;
  if (paramlist->u.decList.isEmpty) { return emptyParamTypes(); }

  pt = checkProcHelperBuildParam(paramlist->u.decList.tail,table);
	  
  param = paramlist->u.decList.head;
  type = checkType(param->u.parDec.ty,table);
  
  if ((type->kind == TYPE_KIND_ARRAY) && (!param->u.parDec.isRef)) {
    error("parameter '%s' must be a reference parameter in line %d",symToString(param->u.parDec.name),param->line);
  }

  prevDec = lookup(table, param->u.parDec.name);
  if (prevDec != NULL && prevDec->kind == ENTRY_KIND_VAR) {
    errorMsgRedeclarationParam(param->u.parDec.name,param->line);
  }

  enter(table, param->u.parDec.name, newVarEntry(type,param->u.parDec.isRef) ); 

  return newParamTypes(type, param->u.parDec.isRef, pt);
}

Entry *checkProc(Absyn *procDec,Table *table,boolean firstLook) {
  Type *type;
  Entry *prevDec;
  Table *localTable;
  /* zwischen erstdurchlauf (nur kopf und parameter durchsuchen) und
  zweitdurchlauf (prozedurrumpf untersuchen) muss unterschieden werden */
  if (firstLook) {
        Absyn *paramlist = procDec->u.procDec.params;
  	ParamTypes *paramtypes = NULL;
        //Absyn *param;

  	localTable = newTable(table);

	/* Parameterlist pruefen */
        paramtypes = checkProcHelperBuildParam(paramlist,localTable);
/* Funktioniert nicht, da Parameter falsch herum in die Tabelle eingelesen werden. 
        ParamTypes *paramtypes = emptyParamTypes();

	while (!paramlist->u.decList.isEmpty) {
	param = paramlist->u.decList.head;
	
	type = checkType(param->u.parDec.ty,table);
	
	prevDec = lookup(localTable, param->u.parDec.name);
	if (prevDec != NULL && prevDec->kind == ENTRY_KIND_VAR) {
          errorMsgRedeclarationParam(param->u.parDec.name,param->line);
	}
	enter(localTable, param->u.parDec.name, newVarEntry(type,param->u.parDec.isRef) ); 
	
	paramtypes = newParamTypes(type, param->u.parDec.isRef, paramtypes);
	
	paramlist = paramlist->u.decList.tail;
	}
*/	
	return newProcEntry(paramtypes, localTable);
  } else {
    Absyn *declist = procDec->u.procDec.decls;
    Absyn *dec;

    /* die im ersten durchlauf generierte Symboltabelle holen */
    Entry *proc = lookup(table, procDec->u.procDec.name);
    localTable = proc->u.procEntry.localTable;

    /* Lokale Variablen pruefen */ 
    while (!declist->u.decList.isEmpty) {
	dec = declist->u.decList.head;
	type = checkType(dec->u.parDec.ty,localTable);
	
	prevDec = lookup(localTable, dec->u.parDec.name);
	if (prevDec != NULL && prevDec->kind == ENTRY_KIND_VAR) {
          errorMsgRedeclarationEntry(dec->u.parDec.name,prevDec,dec->line);
	}
	enter(localTable, dec->u.parDec.name, newVarEntry(type,dec->u.parDec.isRef) ); 
	
	declist = declist->u.decList.tail;
    }
    
    /* Prozedur Rumpf durchlaufen */
    checkStmList(procDec->u.procDec.body,localTable);
    return proc;
  }
}

/*--------------------------------------------------------------------------------------*/

void checkStmList(Absyn *stmlist,Table *table) {
  while (!stmlist->u.stmList.isEmpty) {
    checkStm(stmlist->u.stmList.head,table);
    stmlist = stmlist->u.stmList.tail;
  }
}

/*--------------------------------------------------------------------------------------*/

void checkStm(Absyn *stm,Table *table) {
    switch (stm->type) {
      case ABSYN_EMPTYSTM: break;
      case ABSYN_COMPSTM: checkStmList(stm->u.compStm.stms,table); break;
      case ABSYN_ASSIGNSTM: {
                              Type *type = checkVar(stm->u.assignStm.var,table);
                              if (type->kind != TYPE_KIND_PRIMITIVE) {
				error("assignment requires integer variable in line  %d",stm->line);
			      } 
                              if (type != checkExp(stm->u.assignStm.exp,table)) { 
                                error("assignment has different types in line %d",stm->line);
                              }
                            }
                            break;
      case ABSYN_IFSTM: checkTest(stm->u.ifStm.test,table); 
                        checkStm(stm->u.ifStm.thenPart,table); 
                        checkStm(stm->u.ifStm.elsePart,table); 
                        break;
      case ABSYN_WHILESTM: checkTest(stm->u.whileStm.test,table); 
                           checkStm(stm->u.whileStm.body,table); 
                           break;
      case ABSYN_CALLSTM: { 
                            Entry *proc = lookup(table, stm->u.callStm.name);
                            ParamTypes *paramtypes;
                            Absyn *args;
                            int argc;
                            if (proc == NULL) {
  			      error("undefined procedure '%s' in line %d",symToString(stm->u.callStm.name),stm->line);
			    }
                            if (proc->kind != ENTRY_KIND_PROC) {
                              error("call of non-procedure '%s' in line %d",symToString(stm->u.callStm.name),stm->line);
                            }

                            //Parameter pruefen
                            args = stm->u.callStm.args;
                            paramtypes = proc->u.procEntry.paramTypes;
                            argc = 1;
                            while (!paramtypes->isEmpty && !args->u.expList.isEmpty) {
                              if (paramtypes->isRef) {
                                if ( args->u.expList.head->type != ABSYN_VAREXP ) {
                                  error("procedure '%s' argument %d must be a variable in line %d",symToString(stm->u.callStm.name),argc,stm->line);
                                } 
                              }
                              if (checkExp( args->u.expList.head,table ) != paramtypes->type) { 
			        error("procedure '%s' argument %d type mismatch in line %d",symToString(stm->u.callStm.name),argc,stm->line);
                              }
                              paramtypes = paramtypes->next;
			      args = args->u.expList.tail;
                              argc++;
                            }
                            if (!paramtypes->isEmpty && args->u.expList.isEmpty) {
                              error("procedure '%s' called with too few arguments in line %d",symToString(stm->u.callStm.name),stm->line);
                            }
                            if (!args->u.expList.isEmpty && paramtypes->isEmpty) {
                              error("procedure '%s' called with too many arguments in line %d",symToString(stm->u.callStm.name),stm->line);
                            }
                          }
                          break;
      default : errorMsgUnknownElement(stm->line);
    }
    return;
}

/*--------------------------------------------------------------------------------------*/

Type *checkVar(Absyn *var,Table *table) {
 Type *type;
 switch (var->type) {
   case ABSYN_SIMPLEVAR : {
                            Entry *entry = lookup(table,var->u.simpleVar.name);
                            if (entry == NULL) {
                              error("undefined variable '%s' in line %d",symToString(var->u.simpleVar.name),var->line);
                            }
                            if (entry->kind != ENTRY_KIND_VAR) {
                              error("'%s' is not a variable in line %d",symToString(var->u.simpleVar.name),var->line);
                            } 
                            type = entry->u.varEntry.type;
                          }  
                         break;
   case ABSYN_ARRAYVAR : type = checkVar(var->u.arrayVar.var,table);
                         if (type->kind != TYPE_KIND_ARRAY) {
                           error("illegal indexing a non-array in line %d",var->line);
                         } else {
                           type = type->u.arrayType.baseType;
                         }
                         if (checkExp(var->u.arrayVar.index,table) != intType->u.typeEntry.type) {
                           error("illegal indexing with a non-integer in line %d",var->line);
                         } 
                         break;
   default : errorMsgUnknownElement(var->line);
 }
 return type;
}

/*--------------------------------------------------------------------------------------*/

void checkTest(Absyn *test,Table *table) {
  if (test->type != ABSYN_OPEXP) {
    error("'if'/'while' test expression must be of type boolean in line %d",test->line);		 
  }
  switch (test->u.opExp.op) {
	case ABSYN_OP_EQU :		
	case ABSYN_OP_NEQ :		
	case ABSYN_OP_LST :		
	case ABSYN_OP_LSE : 		
	case ABSYN_OP_GRT :		
	case ABSYN_OP_GRE : { 
			Type *type = checkExp(test->u.opExp.left,table);
			if (type->kind != TYPE_KIND_PRIMITIVE) {
				error("comparison requires integer operands in line %d",test->line);
			} 
			if (type != checkExp(test->u.opExp.right,table)) {
				error("expression combines different types in line %d",test->line);
			}
                       } 
                       break;
	default : error("'if'/'while' test expression must be of type boolean in line %d",test->line);		 
  }
}

/*--------------------------------------------------------------------------------------*/

Type *checkExp(Absyn *exp,Table *table) {
  Type *type;
  switch (exp->type) {
      case ABSYN_OPEXP : switch (exp->u.opExp.op) {
                      case ABSYN_OP_ADD : 
                      case ABSYN_OP_SUB : 
                      case ABSYN_OP_MUL : 
                      case ABSYN_OP_DIV : type = checkExp(exp->u.opExp.left,table);
                                     if (type->kind != TYPE_KIND_PRIMITIVE) {
                                       error("arithmetic operation requires integer operands in line %d",exp->line);
                                     } 
                                     if (type != checkExp(exp->u.opExp.right,table)) {
				       error("expression combines different types in line %d",exp->line);
				     } 
                                     break;
                      default : error("compares are not allowed in arithmetic operations in line %d",exp->line);
                    }
                    break;
      case ABSYN_VAREXP : type = checkVar(exp->u.varExp.var,table);
                     break;
      case ABSYN_INTEXP : type = intType->u.typeEntry.type; 
                     break;
      default :errorMsgUnknownElement(exp->line);
  }
  return type;
}

void writeDefaultEntrys(Table *globalTable) {
  Table *localTable;
  ParamTypes *paramtypes;

  //Lokale Tabelle wird immer leergelassen. 
  localTable = newTable(globalTable);

  //Type: int
  enter(globalTable, newSym("int"), intType);
  
  //exit()
  enter(globalTable, newSym("exit"), newProcEntry(paramtypes, localTable));
  
  paramtypes = emptyParamTypes();
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);

  //printi(i: int)
  enter(globalTable, newSym("printi"), newProcEntry(paramtypes, localTable));
  //printc(i: int)
  enter(globalTable, newSym("printc"), newProcEntry(paramtypes, localTable));
  //clearAll(i: int)
  enter(globalTable, newSym("clearAll"), newProcEntry(paramtypes, localTable));

  paramtypes = emptyParamTypes();
  paramtypes = newParamTypes(intType->u.typeEntry.type, TRUE, paramtypes);

  //readi(ref i: int)
  enter(globalTable, newSym("readi"), newProcEntry(paramtypes, localTable));
  //readc(ref i: int)
  enter(globalTable, newSym("readc"), newProcEntry(paramtypes, localTable));
  //time(ref i: int)
  enter(globalTable, newSym("time"), newProcEntry(paramtypes, localTable));

  paramtypes = emptyParamTypes();
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);

  //setPixel(x: int, y: int, color: int)
  enter(globalTable, newSym("setPixel"), newProcEntry(paramtypes, localTable));
  
  paramtypes = emptyParamTypes();
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);

  //drawLine(x1: int, y1: int, x2: int, y2: int, color: int)
  enter(globalTable, newSym("drawLine"), newProcEntry(paramtypes, localTable));

  paramtypes = emptyParamTypes();
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);
  paramtypes = newParamTypes(intType->u.typeEntry.type, FALSE, paramtypes);

  //drawCircle(x0: int, y0: int, radius: int, color: int)
  enter(globalTable, newSym("drawCircle"), newProcEntry(paramtypes, localTable));
}

/*######################################################################################*/
/* Implementierung der Fehlerausgaben */

char *errorMsgType = "type";
char *errorMsgProcedure = "procedure";
char *errorMsgParameter = "parameter";
char *errorMsgVariable = "variable";

void errorMsgRedeclarationEntry(Sym *sym,Entry *entry,int line) {
  char *type;
  switch (entry->kind) {
   case ENTRY_KIND_TYPE : type = errorMsgType; break;
   case ENTRY_KIND_VAR : type = errorMsgVariable; break;
   case ENTRY_KIND_PROC : type = errorMsgProcedure; break;
   default : error("datatypes not consistent");
  }
  errorMsgRedeclaration(sym,type,line);
}
void errorMsgRedeclarationParam(Sym *sym,int line) {
  errorMsgRedeclaration(sym,errorMsgParameter,line);
}
void errorMsgRedeclaration(Sym *sym,char *type,int line) {
  error("redeclaration of '%s' as %s in line %d",symToString(sym),type,line);
}

void errorMsgUnknownElement(int line) {
  error("unknown element in line %d",line);
}

/*######################################################################################*/
