%{

/*
 * parser.y -- SPL parser specification
 */

#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "common.h"
#include "utils.h"
#include "scanner.h"
#include "parser.h"

#include "sym.h"
#include "absyn.h"

Absyn *progTree;


/*
#define YYSTYPE Absyn*
%union {
  NoVal noVal;
  IntVal intVal;
  StringVal stringVal;
  Absyn *node;
}
*/

%}

%union {
  NoVal noVal;
  IntVal intVal;
  StringVal stringVal;
 Sym *sym;
 Absyn *absyn;
}

%debug

%token CLASS
%token ARRAY
%token ELSE
%token IF
%token OF
%token PROC
%token REF
%token TYPE
%token VAR
%token WHILE
%token LPAREN
%token RPAREN
%token LBRACK
%token RBRACK
%token LCURL
%token RCURL
%token EQ
%token NE
%token LT
%token LE
%token GT
%token GE
%token ASGN
%token COLON
%token COMMA
%token SEMIC
%token PLUS
%token MINUS
%token STAR
%token SLASH
%token IDENT
%token INTLIT


%type<intVal> INTLIT
%type<sym> name
%type<absyn> start program typedeflist typedef function typ paramlist paramlist1 param deklarationlist deklaration statemantlist statemant lhs rhs procedurecall argumentlist argumentlist1 exp boolexp mathexp term faktor var intexp varexp

%start			start

%%

start			:       program { progTree = $1; }

program			: 	typedef program { $$ = newDecList( $1,$2 ); }
				| 	function program { $$ = newDecList( $1,$2 ); }
				| 	function typedeflist { $$ = newDecList( $1,$2 ); }
				| 	function { $$ = newDecList($1,emptyDecList()); }
			;

typedeflist		: 	typedef typedeflist { $$ = newDecList( $1,$2 ); }
				| 	typedef { $$ = newDecList($1,emptyDecList()); }
			;


typedef			: 	TYPE name EQ typ SEMIC { $$ = newTypeDec(yylval.noVal.line, $2, $4); }
			;

typ			: 	IDENT { $$ = newNameTy(yylval.noVal.line,newSym(yytext)); }
				| ARRAY LBRACK INTLIT RBRACK OF typ { $$ = newArrayTy(yylval.noVal.line,$3.val,$6); }
			;

name			:	IDENT { $$ = newSym(yytext); }
			;

function		: 	PROC name LPAREN paramlist RPAREN LCURL deklarationlist statemantlist RCURL { $$ = newProcDec(yylval.noVal.line,$2,$4,$7,$8); }

			;

paramlist		:	/* empty */ { $$ = emptyDecList();}
				| paramlist1 { $$ = $1; }
			;

paramlist1		:	param { $$ = newDecList($1,emptyDecList()); }
				|   param COMMA paramlist1 { $$ = newDecList($1,$3);  }
			;

param			:   	name COLON typ { $$ = newParDec(yylval.noVal.line,$1,$3,FALSE); }
				| REF name COLON typ { $$ = newParDec(yylval.noVal.line,$2,$4,TRUE); }
			;

deklarationlist		: 	/* empty */ { $$ = emptyDecList(); }
				| deklaration deklarationlist { $$ = newDecList($1,$2); }
			;

deklaration		: VAR name COLON typ SEMIC { $$ = newVarDec(yylval.noVal.line, $2, $4); }
			;

statemantlist		:	/* empty */ { $$ = emptyStmList(); }
				|  statemant statemantlist { $$ = newStmList($1,$2); }
			;

statemant		:	SEMIC /* die leere Anweisung */ { $$ = newEmptyStm(yylval.noVal.line); }
				| lhs ASGN rhs SEMIC /* Zuweisung */ { $$ = newAssignStm(yylval.noVal.line,$1,$3); }
				| IF LPAREN exp RPAREN statemant /* Einarmiges IF */ { $$ = newIfStm(yylval.noVal.line,$3,$5,newEmptyStm(yylval.noVal.line)); }
				| IF LPAREN exp RPAREN statemant ELSE statemant /* Zweiarmiges IF */ { $$ = newIfStm(yylval.noVal.line,$3,$5,$7); }
				| WHILE LPAREN exp RPAREN statemant /* abweisende Schleife */ { $$ = newWhileStm(yylval.noVal.line,$3,$5); }
				| LCURL statemantlist RCURL /* Anweisungsblock */ { $$ = newCompStm(yylval.noVal.line,$2); }
				| procedurecall SEMIC { $$ = $1; }
			;


lhs			:	var { $$ = $1; }
			;

rhs			:	exp { $$ = $1; }
			;


procedurecall		:	name LPAREN argumentlist RPAREN { $$ = newCallStm(yylval.noVal.line,$1,$3); }
			;

argumentlist		:	/* empty */ { $$ = emptyExpList(); }
				| argumentlist1 { $$ = $1; }
			;

argumentlist1		:	mathexp { $$ = newExpList($1,emptyExpList()); }
				| mathexp COMMA argumentlist1 { $$ = newExpList($1,$3); }
			;
exp			:	boolexp { $$ = $1; }
			;

boolexp			:	mathexp EQ mathexp { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_EQU,$1,$3); }
				| mathexp NE mathexp { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_NEQ,$1,$3); }
				| mathexp LT mathexp { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_LST,$1,$3); }
				| mathexp LE mathexp { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_LSE,$1,$3); }
				| mathexp GT mathexp { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_GRT,$1,$3); }
				| mathexp GE mathexp { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_GRE,$1,$3); }
				| mathexp { $$ = $1; }
			;

mathexp			:	mathexp PLUS term { $$ =  newOpExp(yylval.noVal.line,ABSYN_OP_ADD,$1,$3); }
				| mathexp MINUS term { $$ =  newOpExp(yylval.noVal.line,ABSYN_OP_SUB,$1,$3); }
				| term  { $$ = $1; }
			;

term			:	term STAR faktor { $$ =  newOpExp(yylval.noVal.line,ABSYN_OP_MUL,$1,$3); }
				| term SLASH faktor { $$ =  newOpExp(yylval.noVal.line,ABSYN_OP_DIV,$1,$3); }
				| faktor  { $$ = $1; }
			;

faktor			:	MINUS faktor { $$ = newOpExp(yylval.noVal.line,ABSYN_OP_SUB,newIntExp(yylval.noVal.line,0),$2);}
				| intexp { $$ = $1; }
				| LPAREN boolexp RPAREN { $$ = $2; }
				| varexp { $$ = $1; }
			;

varexp		:       var { $$ = newVarExp(yylval.noVal.line,$1); }
			;


var			: 	name { $$ = newSimpleVar(yylval.noVal.line,$1); }
				| var LBRACK exp RBRACK { $$ = newArrayVar(yylval.noVal.line,$1,$3); }
			;

intexp		: 	INTLIT { $$ = newIntExp(yylval.noVal.line,$1.val); }
//
//				{
//					int intvalue = 0;
//					int i;
//					/* Versuchen wir den normalen integer */
//					sscanf(yytext,"%d",&intvalue);
//					if (intvalue == 0) {
//						/* Versuchen wir den character */
//						if (sscanf(yytext,"'%c'",(char*)&intvalue) == 0) {
//						   /* versuchen wir hexadezimalschreibweise */
//						   for (i= 0;i < strlen(yytext);i++) {
//							yytext[i] = tolower(yytext[i]);
//						   }
//						   sscanf(yytext,"0x%x",&intvalue);
//						}
//					}
//					$$ = newIntExp(yylval.noVal.line,intvalue);
//				}
//
			;

%%


void yyerror(char *msg) {
 error("%s in line %d", msg, yylval.noVal.line);
}
