/*
 * types.c -- type representation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "types.h"
#include "sym.h"

static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}

void showSideEntry(Entry *side, int n) {
    indent(n);printf("GroupIDCounter  : %d\n",side->u.sideEntry.groupIdCounter);
    showTable(side->u.sideEntry.groupTable,++n);
}

void showGroupEntry(Entry *group, int n) {
    indent(n);printf("GroupID         : %d\n",group->u.groupEntry.groupId);
    showTable(group->u.groupEntry.unitTable,++n);
}

void showUnitEntry(Entry *unit, int n) {
    indent(n);printf("UnitId          : %s\n",unit->u.unitEntry.unitId);
    indent(n);printf("RankName        : %s\n",unit->u.unitEntry.rankName);
    indent(n);printf("RankShortName   : %s\n",unit->u.unitEntry.rankShortName);
    indent(n);printf("Class           : %s\n",symToString(unit->u.unitEntry.classname));
    indent(n);printf("Classname       : %s\n",symToValue(unit->u.unitEntry.classname));
    indent(n);printf("isLeader        : %d\n",unit->u.unitEntry.isLeader);
    indent(n);printf("description     : %s\n",unit->u.unitEntry.description);
    indent(n);printf("position        : %s\n",unit->u.unitEntry.position);
}


