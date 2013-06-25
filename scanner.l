%{

/*
 * scanner.l -- SPL scanner specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"
#include "utils.h"
#include "scanner.h"

#include "sym.h"
#include "absyn.h"

#include "parser.tab.h"
#include "parser.h"

#define TKRETURN(TK) { yylval.noVal.line = lineNumber; return (TK); }

static int lineNumber = 1;

%}

CLASS       class
EQ          =
LCURL       \{
RCURL       \}
COMMA       ,
SEMIC       ;
PLUS        \+
MINUS       -
LBRACK      \[
RBRACK      \]
IDENT       [a-zA-Z_][a-zA-Z0-9_]*
NUMBERLIT_DEC  [0-9]+("."[0-9]+([eE][-+]?[0-9]+)?)?
NUMBERLIT_HEX  0x[a-fA-F0-9]+
STRINGLIT   "\"".*"\""
WHITESPACE  [ \t]+
LINEBREAK   \n
LINEBREAK_DOS   \r
ANYCHAR     .

%%

{LINEBREAK}     lineNumber++;
{LINEBREAK_DOS} lineNumber++;
{WHITESPACE}    { /* Keine Leerzeichen und Tabs */ }

{CLASS}         TKRETURN(CLASS);
{EQ}            TKRETURN(EQ);
{LCURL}         TKRETURN(LCURL);
{RCURL}         TKRETURN(RCURL);
{COMMA}         TKRETURN(COMMA);
{SEMIC}         TKRETURN(SEMIC);
{PLUS}          TKRETURN(PLUS);
{MINUS}         TKRETURN(MINUS);
{LBRACK}        TKRETURN(LBRACK);
{RBRACK}        TKRETURN(RBRACK);
{IDENT}         {	
                  yylval.StringVal.val = yytext;
                  TKRETURN(IDENT);
                }
{NUMBERLIT_DEC} {	
                  yylval.StringVal.val = yytext;
                  TKRETURN(NUMBERLIT_DEC); 
                }
{NUMBERLIT_HEX} {	
                  yylval.StringVal.val = yytext;
                  TKRETURN(NUMBERLIT_HEX); 
                }
{STRINGLIT}     {	
                  yylval.StringVal.val = yytext;
                  TKRETURN(STRINGLIT);
                }
{ANYCHAR}       { error("\nFehler in Zeile %d: Ungueltiges Zeichen 0x%X \n",lineNumber,yytext[0]); }


%%



int yywrap(void) {
  return 1;
}


void showToken(int token) {
	switch(token) {
		case 0: printf(" EOF \n");
			break;
		case CLASS: printf(" CLASS ");
					break;
	    case EQ:    printf(" EQ ");
					break;
	    case LCURL: printf(" LCURL \n");
					break;
	    case RCURL: printf(" RCRUL \n");
					break;
		case COMMA: printf(" COMMA ");
					break;
		case SEMIC: printf(" SEMIC \n");
					break;
		case PLUS:  printf(" PLUS ");
					break;
		case MINUS: printf(" MINUS ");
					break;
	    case LBRACK: printf(" LBRACK");
					break;
	    case RBRACK: printf(" RBRACK ");
                    break;
		case IDENT: printf(" IDENT(\"%s\") ",yytext);
					break;
		case NUMBERLIT_DEC: printf(" NUMBERLIT_DEC(\"%s\") ",yytext);
					break;
		case NUMBERLIT_HEX: printf(" NUMBERLIT_HEX(\"%s\") ",yytext);
					break;
		case STRINGLIT: printf(" STRINGLIT(\"%s\") ",yytext);
					break;
		default: printf(" UNBEKANNTER TOKEN! %d \n",token);
	}
}
