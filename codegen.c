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


void genCodeSides(Bintree *bintree, FILE *outFile);
void genCodeGroups(Bintree *bintree, FILE *outFile);
boolean checkAtLeastOneGroupFill(Bintree *bintree);
void genCodeGroup(Sym *name, Entry *group, FILE *outFile);
void genCodeUnits(Bintree *bintree, FILE *outFile, int *unitId);
void genCodeUnit(Sym *name, Entry *unit,FILE *outFile, int unitId);

void genCode(Table *globalTable, FILE *outFile) {
  fprintf(outFile, "$result = array();\n");
  genCodeSides(globalTable->bintree,outFile);
  fprintf(outFile, "return $result;\n");
}

void genCodeSides(Bintree *bintree, FILE *outFile) {
  if (bintree == NULL) {
    return;
  }
  genCodeSides(bintree->left,outFile);
  fprintf(outFile, "$result['%s'] = array(\n",symToString(bintree->sym));

  if (bintree->entry->u.sideEntry.groupTable->size && checkAtLeastOneGroupFill(bintree->entry->u.sideEntry.groupTable->bintree)) {
    genCodeGroups(bintree->entry->u.sideEntry.groupTable->bintree,outFile);
  }
  fprintf(outFile, ");\n");
  genCodeSides(bintree->right,outFile);
}

boolean checkAtLeastOneGroupFill(Bintree *bintree) {
   if (bintree == NULL) {
     return FALSE;
   }
   return checkAtLeastOneGroupFill(bintree->left) || (bintree->entry->u.groupEntry.unitTable->size > 0) || checkAtLeastOneGroupFill(bintree->right);
}

void genCodeGroups(Bintree *bintree, FILE *outFile) {
  if (bintree == NULL) {
    return;
  }
  genCodeGroups(bintree->left,outFile);
  if (bintree->entry->u.groupEntry.unitTable->size) {
    genCodeGroup(bintree->sym, bintree->entry,outFile);
  }
  genCodeGroups(bintree->right,outFile);
}

void genCodeGroup(Sym *name,Entry *group,FILE *outFile) {
 int unitStartId = 0;
 fprintf(outFile, "  array(\n");
 fprintf(outFile, "    'name'    => '%s',\n",symToValue(name));
 fprintf(outFile, "    'squadId' => %d,\n",group->u.groupEntry.groupId);
 fprintf(outFile, "    'slots'   => array(\n");
 genCodeUnits(group->u.groupEntry.unitTable->bintree,outFile,&unitStartId);
 fprintf(outFile, "    ),\n");
 fprintf(outFile, "  ),\n");
}

void genCodeUnits(Bintree *bintree, FILE *outFile, int *unitId) {
  if (bintree == NULL) {
    return;
  }
  genCodeUnits(bintree->left,outFile,unitId);
  genCodeUnit(bintree->sym, bintree->entry,outFile, *unitId);
  genCodeUnits(bintree->right,outFile,unitId);
}

void genCodeUnit(Sym *name, Entry *unit,FILE *outFile, int unitId) {
    fprintf(outFile,"      array(\n");
    fprintf(outFile,"        'id'           => '%s',\n",unit->u.unitEntry.unitId);
    fprintf(outFile,"        'groupId'      => %d,\n",unitId);
    fprintf(outFile,"        'rank'         => '%s',\n",unit->u.unitEntry.rank);
    fprintf(outFile,"        'rankName'     => '%s',\n",unit->u.unitEntry.rankName);
    fprintf(outFile,"        'rankShortName'=> '%s',\n",unit->u.unitEntry.rankShortName);
    fprintf(outFile,"        'class'        => '%s',\n",symToString(unit->u.unitEntry.classname));
    fprintf(outFile,"        'classname'    => '%s',\n",symToValue(unit->u.unitEntry.classname));
    fprintf(outFile,"        'isLeader'     => %d,\n",unit->u.unitEntry.isLeader);
    fprintf(outFile,"        'description'  => '%s',\n",unit->u.unitEntry.description);
    fprintf(outFile,"        'position'     => '%s',\n",unit->u.unitEntry.position);
    fprintf(outFile,"      ),\n");

}
