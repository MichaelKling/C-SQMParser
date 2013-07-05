/*
 * absyn.h -- abstract syntax
 */


#ifndef _ABSYN_H_
#define _ABSYN_H_

#define ABSYN_NUMTY	    	0
#define ABSYN_STRTY	    	1
#define ABSYN_CLASSTY	  	2
#define ABSYN_ARRAYTY    	3
#define ABSYN_DECLIST       4
#define ABSYN_STRLIST       5
#define ABSYN_VALLIST       6

#define ABSYN_NUM	    	7
#define ABSYN_STR	    	8

typedef struct absyn {
  int type;
  int line;
  union {
    struct {
      Sym *value;
    } num;
    struct {
      Sym *value;
    } str;
    struct {
      Sym *name;
      struct absyn *value;
    } numTy;
    struct {
      Sym *name;
      struct absyn *strList;
    } strTy;
    struct {
      Sym *name;
      struct absyn *decList;
    } classTy;
    struct {
      Sym *name;
      struct absyn *valList;
    } arrayTy;
    struct {
      boolean isEmpty;
      struct absyn *head;
      struct absyn *tail;
    } decList;
    struct {
      boolean isEmpty;
      struct absyn *head;
      struct absyn *tail;
    } strList;
    struct {
      boolean isEmpty;
      struct absyn *head;
      struct absyn *tail;
    } valList;
  } u;
} Absyn;


Absyn *newNum(int line, Sym *value);
Absyn *newStr(int line, Sym *value);
Absyn *newNumTy(int line, Sym *name, Absyn *value);
Absyn *newStrTy(int line, Sym *name, Absyn *strList);
Absyn *newClassTy(int line, Sym *name, Absyn *decList);
Absyn *newArrayTy(int line, Sym *name, Absyn *valList);
Absyn *emptyDecList(void);
Absyn *newDecList(Absyn *head, Absyn *tail);
Absyn *emptyStrList(void);
Absyn *newStrList(Absyn *head, Absyn *tail);
Absyn *emptyValList(void);
Absyn *newValList(Absyn *head, Absyn *tail);

char *strListToString(Absyn *strList);

void showAbsyn(Absyn *node);


#endif /* _ABSYN_H_ */
