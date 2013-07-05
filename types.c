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


void showSideEntry(Entry *side) {
    printf("GroupIDCounter  : %d",side->u.sideEntry.groupIdCounter);
    showTable(side->u.sideEntry.groupTable);
}

void showGroupEntry(Entry *group) {
    printf("GroupID         : %d",group->u.groupEntry.groupId);
    printf("GroupIDSecId    : %d",group->u.groupEntry.groupSecondaryId);
    showTable(group->u.groupEntry.unitTable);
}

void showUnitEntry(Entry *unit) {
    printf("GroupID         : %d",unit->u.unitEntry.groupId);
    printf("UnitId          : %s",unit->u.unitEntry.unitId);
    printf("RankName        : %s",unit->u.unitEntry.rankName);
    printf("RankShortName   : %s",unit->u.unitEntry.rankShortName);
    printf("Class           : %s",symToString(unit->u.unitEntry.classname));
    printf("Classname       : %s",symToValue(unit->u.unitEntry.classname));
    printf("isLeader        : %d",unit->u.unitEntry.isLeader);
    printf("description     : %s",unit->u.unitEntry.description);
    printf("position        : %s",unit->u.unitEntry.position);
}


