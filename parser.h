/*
 * parser.h -- SPL parser interface
 */


#ifndef _PARSER_H_
#define _PARSER_H_

#include "sym.h"
#include "absyn.h"

int yyparse(void);
void yyerror(char *msg);

extern Absyn *progTree;
extern int yydebug;

#endif /* _PARSER_H_ */
