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
    printf("\tGroupIDCounter  : %d\n",side->u.sideEntry.groupIdCounter);
    showTable(side->u.sideEntry.groupTable);
}

void showGroupEntry(Entry *group) {
    printf("\tGroupID         : %d\n",group->u.groupEntry.groupId);
    printf("\tGroupIDSecId    : %d\n",group->u.groupEntry.groupSecondaryId);
    showTable(group->u.groupEntry.unitTable);
}

void showUnitEntry(Entry *unit) {
    printf("\tGroupID         : %d\n",unit->u.unitEntry.groupId);
    printf("\tUnitId          : %s\n",unit->u.unitEntry.unitId);
    printf("\tRankName        : %s\n",unit->u.unitEntry.rankName);
    printf("\tRankShortName   : %s\n",unit->u.unitEntry.rankShortName);
    printf("\tClass           : %s\n",symToString(unit->u.unitEntry.classname));
    printf("\tClassname       : %s\n",symToValue(unit->u.unitEntry.classname));
    printf("\tisLeader        : %d\n",unit->u.unitEntry.isLeader);
    printf("\tdescription     : %s\n",unit->u.unitEntry.description);
    printf("\tposition        : %s\n",unit->u.unitEntry.position);
}


