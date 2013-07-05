/*
 * absyn.c -- abstract syntax
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"


/**************************************************************/

Absyn *newNum(int line, Sym *value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_NUM;
  node->line = line;
  node->u.num.value = value;
  return node;
}

Absyn *newStr(int line, Sym *value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STR;
  node->line = line;
  node->u.str.value = value;
  return node;
}

Absyn *newNumTy(int line, Sym *name, Absyn *value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_NUMTY;
  node->line = line;
  node->u.numTy.name = name;
  node->u.numTy.value = value;
  return node;
}

Absyn *newStrTy(int line, Sym *name, Absyn *strList) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STRTY;
  node->line = line;
  node->u.strTy.name = name;
  node->u.strTy.strList = strList;
  return node;
}

Absyn *newClassTy(int line, Sym *name, Absyn *decList) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CLASSTY;
  node->line = line;
  node->u.classTy.name = name;
  node->u.classTy.decList = decList;
  return node;
}

Absyn *newArrayTy(int line, Sym *name, Absyn *valList) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ARRAYTY;
  node->line = line;
  node->u.arrayTy.name = name;
  node->u.arrayTy.valList = valList;
  return node;
}

Absyn *emptyDecList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_DECLIST;
  node->line = -1;
  node->u.decList.isEmpty = TRUE;
  return node;
}


Absyn *newDecList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_DECLIST;
  node->line = -1;
  node->u.decList.isEmpty = FALSE;
  node->u.decList.head = head;
  node->u.decList.tail = tail;
  return node;
}

Absyn *emptyStrList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STRLIST;
  node->line = -1;
  node->u.strList.isEmpty = TRUE;
  return node;
}


Absyn *newStrList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STRLIST;
  node->line = -1;
  node->u.strList.isEmpty = FALSE;
  node->u.strList.head = head;
  node->u.strList.tail = tail;
  return node;
}

Absyn *emptyValList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VALLIST;
  node->line = -1;
  node->u.valList.isEmpty = TRUE;
  return node;
}


Absyn *newValList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VALLIST;
  node->line = -1;
  node->u.valList.isEmpty = FALSE;
  node->u.valList.head = head;
  node->u.valList.tail = tail;
  return node;
}


char *strListToString(Absyn *strList) {
    Absyn *node,*dec;
    unsigned strLength;
    unsigned strPos;
    char *newStr;

    strLength = 0;
    strPos = 0;
    node = strList;

    if (node->u.strList.isEmpty) {
        return "";
    }

    if (node->u.strList.tail->u.strList.isEmpty) {
        return symToString(node->u.strList.head->u.str.value);
    }

    while (!node->u.strList.isEmpty) {
        dec = node->u.strList.head;
        strLength += strlen(symToString(dec->u.str.value));
        node = node->u.strList.tail;
        if (!node->u.strList.isEmpty) {
            strLength += 1;
        }
    }
    newStr = (char *) allocate((strLength +1) * sizeof(char));

    node = strList;
    while (!node->u.strList.isEmpty) {
        dec = node->u.strList.head;
        strcpy(newStr + strPos,symToString(dec->u.str.value));
        strPos += strlen(symToString(dec->u.str.value));
        node = node->u.strList.tail;
        if (!node->u.strList.isEmpty) {
            newStr[++strPos] = '\"';
        }
    }
    newStr[++strPos] = 0;
    return newStr;
}

/**************************************************************/


static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}


static void say(char *s) {
  printf("%s", s);
}


static void sayInt(int i) {
  printf("%d", i);
}


static void sayBoolean(boolean b) {
  if (b) {
    printf("true");
  } else {
    printf("false");
  }
}


static void showNode(Absyn *node, int indent);




static void showNum(Absyn *node, int n) {
  indent(n);
  say("Num(");
  say(symToString(node->u.num.value));
  say(")");
}

static void showStr(Absyn *node, int n) {
  indent(n);
  say("Str(");
  say(symToString(node->u.str.value));
  say(")");
}

static void showNumTy(Absyn *node, int n) {
  indent(n);
  say("NumTy(\n");
  indent(n + 1);
  say(symToString(node->u.numTy.name));
  say(",\n");
  showNode(node->u.numTy.value, n + 1);
  say(")");
}

static void showStrTy(Absyn *node, int n) {
  indent(n);
  say("StrTy(\n");
  indent(n + 1);
  say(symToString(node->u.strTy.name));
  say(",\n");
  showNode(node->u.strTy.strList, n + 1);
  say(")");
}

static void showClassTy(Absyn *node, int n) {
  indent(n);
  say("ClassTy(\n");
  indent(n + 1);
  say(symToString(node->u.classTy.name));
  say(",\n");
  showNode(node->u.classTy.decList, n + 1);
  say(")");
}

static void showArrayTy(Absyn *node, int n) {
  indent(n);
  say("ArrayTy(\n");
  indent(n + 1);
  say(symToString(node->u.arrayTy.name));
  say(",\n");
  showNode(node->u.arrayTy.valList, n + 1);
  say(")");
}

static void showDecList(Absyn *node, int n) {
  indent(n);
  say("DecList(");
  while (!node->u.decList.isEmpty) {
    say("\n");
    showNode(node->u.decList.head, n + 1);
    node = node->u.decList.tail;
    if (!node->u.decList.isEmpty) {
      say(",");
    }
  }
  say(")");
}

static void showStrList(Absyn *node, int n) {
  indent(n);
  say("StrList(");
  say("\n");
  say(strListToString(node));
  say("\n");
  while (!node->u.decList.isEmpty) {
    say("\n");
    showNode(node->u.decList.head, n + 1);
    node = node->u.decList.tail;
    if (!node->u.decList.isEmpty) {
      say(",");
    }
  }
  say(")");

}

static void showValList(Absyn *node, int n) {
  indent(n);
  say("ValList(");
  while (!node->u.decList.isEmpty) {
    say("\n");
    showNode(node->u.decList.head, n + 1);
    node = node->u.decList.tail;
    if (!node->u.decList.isEmpty) {
      say(",");
    }
  }
  say(")");
}

static void showNode(Absyn *node, int indent) {
  if (node == NULL) {
    error("showNode got NULL node pointer");
  }
  switch (node->type) {
    case ABSYN_NUM:
      showNum(node, indent);
      break;
    case ABSYN_STR:
      showStr(node, indent);
      break;
    case ABSYN_NUMTY:
      showNumTy(node, indent);
      break;
    case ABSYN_STRTY:
      showStrTy(node, indent);
      break;
    case ABSYN_CLASSTY:
      showClassTy(node, indent);
      break;
    case ABSYN_ARRAYTY:
      showArrayTy(node, indent);
      break;
    case ABSYN_DECLIST:
      showDecList(node, indent);
      break;
    case ABSYN_STRLIST:
      showStrList(node, indent);
      break;
    case ABSYN_VALLIST:
      showValList(node, indent);
      break;
    default:
      error("unknown node type %d in showAbsyn", node->type);
  }
}


void showAbsyn(Absyn *node) {
  showNode(node, 0);
  printf("\n");
}
