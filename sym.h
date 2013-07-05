/*
 * sym.h -- symbol management
 */


#ifndef _SYM_H_
#define _SYM_H_


#define INITIAL_HASH_SIZE	2000	/* will be increased to next prime */

#define SYM_NOTYPE 0
#define SYM_VEHICLE 1
#define SYM_MEN 2
#define SYM_SITE 3
#define SYM_ROLE 4


typedef struct sym {
  char *string;			/* external representation of symbol */
  char *value;          /* optional value of sym, defaults to null */
  unsigned type;        /* optional type of sym, defaults to SYM_NOTYPE */
  unsigned stamp;		/* unique random stamp for external use */
  unsigned counter;      /* unique counted number for external use */
  unsigned hashValue;		/* hash value of string, internal use */
  struct sym *next;		/* symbol chaining, internal use */
} Sym;


Sym *newTypedSym(char *string,char *value,unsigned type);
Sym *newSym(char *string);
char *symToString(Sym *sym);
char *symToValue(Sym *sym);
unsigned symToType(Sym *sym);
unsigned symToStamp(Sym *sym);
unsigned symToCounter(Sym *sym);

#endif /* _SYM_H_ */
