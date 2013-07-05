/*
 * semant.c -- semantic analysis
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
#include "semant.h"
#include "classnames/classnames.h"

void errorMsgUnknownElement(int line);

Sym *missionClass, *groupsClass, *vehiclesClass, *itemId, *itemRank, *itemClass, *itemLeader, *itemDescription, *itemSide, *itemPlayer;

Entry *civilian, *blufor, *opfor, *independent;

Table *check(Absyn *program, boolean showSymbolTables);
void checkMission(Absyn *mission,Table *table);
void checkGroups(Absyn *groups,Table *table);
void checkGroupItem(Absyn *groupItem,Table *table);
void checkVehicles(Absyn *vehicles,Table *table);
void checkVehicleItem(Absyn *vehicleItem,Table *table);



Table *check(Absyn *program, boolean showSymbolTables) {
  Table *globalTable = NULL;
  Absyn *node,*dec;

  missionClass = newSym("Mission");
  groupsClass = newSym("Groups");
  vehiclesClass = newSym("Vehicles");
  itemId = newSym("id");
  itemRank = newSym("rank");
  itemClass = newSym("vehicle");
  itemLeader = newSym("leader");
  itemDescription = newSym("description");
  itemPlayer = newSym("player");
  itemSide = newSym("side");

  globalTable = newTable(NULL);

  civilian = newSideEntry(0,newTable(globalTable));
  enter(globalTable,newSym("CIVI"),civilian);
  blufor = newSideEntry(0,newTable(globalTable));
  enter(globalTable,newSym("WEST"),blufor);
  opfor = newSideEntry(0,newTable(globalTable));
  enter(globalTable,newSym("EAST"),opfor);
  independent = newSideEntry(0,newTable(globalTable));
  enter(globalTable,newSym("GUER"),independent);

  // Globale Symboltabelle erstellen + semantik Analyse
  node = program;
  while (!node->u.decList.isEmpty) {
    dec = node->u.decList.head;
    switch (dec->type) {
      case ABSYN_CLASSTY : if (symToStamp(dec->u.classTy.name) == symToStamp(missionClass)) {
                                checkMission(dec,globalTable);
                           }
                           //Else ignore
                           break;

      case ABSYN_ARRAYTY :
      case ABSYN_STRTY :
      case ABSYN_NUMTY : break; //Just ignore these.
      default : errorMsgUnknownElement(dec->line);
    }
    node = node->u.decList.tail;
  }

   // Ausgeben der Tabelle
   if (showSymbolTables) {
     printf("symbol table:\n");
     showTable(globalTable,0);
   }

  // return global symbol table
  return globalTable;
}

void checkMission(Absyn *mission,Table *table) {
    Absyn *node,*dec;
    Table *unitTable;

    node = mission->u.classTy.decList;
    while (!node->u.decList.isEmpty) {
        dec = node->u.decList.head;
        switch (dec->type) {
            case ABSYN_CLASSTY : if (symToStamp(dec->u.classTy.name) == symToStamp(groupsClass)) {
                                    checkGroups(dec,table);
                                 } else if (symToStamp(dec->u.classTy.name) == symToStamp(vehiclesClass)) {



                                    unitTable = newTable(independent->u.sideEntry.groupTable);
                                    independent->u.sideEntry.groupIdCounter++;
                                    enter(independent->u.sideEntry.groupTable,  newSym(classnamesGetNatoAlphabet(independent->u.sideEntry.groupIdCounter)), newGroupEntry(independent->u.sideEntry.groupIdCounter,unitTable));

                                    checkVehicles(dec,unitTable);
                                 }
                               //Else ignore
                               break;

            case ABSYN_ARRAYTY :
            case ABSYN_STRTY :
            case ABSYN_NUMTY : break; //Just ignore these.
            default : errorMsgUnknownElement(dec->line);
        }
        node = node->u.decList.tail;
    }
}

void checkGroups(Absyn *groups,Table *table) {
    Absyn *node,*dec;

    node = groups->u.classTy.decList;
    while (!node->u.decList.isEmpty) {
        dec = node->u.decList.head;
        switch (dec->type) {
            case ABSYN_CLASSTY :
                               checkGroupItem(dec,table);
                               break;

            case ABSYN_ARRAYTY :
            case ABSYN_STRTY :
            case ABSYN_NUMTY : break; //Just ignore these.
            default : errorMsgUnknownElement(dec->line);
        }
        node = node->u.decList.tail;
    }
}

void checkGroupItem(Absyn *groupItem,Table *table) {
    Absyn *node,*dec;
    Table *unitTable;
    Entry *sideEntry;
    char *side = 0;

    node = groupItem->u.classTy.decList;
    while (!node->u.decList.isEmpty) {
        dec = node->u.decList.head;
        switch (dec->type) {
            case ABSYN_CLASSTY :
                               if (symToStamp(dec->u.classTy.name) == symToStamp(vehiclesClass)) {
                                    if (!side) {
                                        errorMsgUnknownElement(dec->line);
                                    } else {
                                        checkVehicles(dec,unitTable);
                                    }

                               }
                               break;

            case ABSYN_ARRAYTY :
            case ABSYN_NUMTY : break; //Just ignore these.
            case ABSYN_STRTY : if (symToStamp(dec->u.strTy.name) == symToStamp(itemSide)) {
                                    side = strListToString(dec->u.strTy.strList);
                                    if (strcmp(side,"WEST") == 0) {
                                        sideEntry = blufor;
                                    } else if (strcmp(side,"EAST") == 0) {
                                        sideEntry = opfor;
                                    } else if (strcmp(side,"GUER") == 0) {
                                        sideEntry = independent;
                                    } else {
                                        sideEntry = civilian;
                                    }
                                    unitTable = newTable(sideEntry->u.sideEntry.groupTable);
                                    sideEntry->u.sideEntry.groupIdCounter++;
                                    enter(sideEntry->u.sideEntry.groupTable,  newSym(classnamesGetNatoAlphabet(sideEntry->u.sideEntry.groupIdCounter)), newGroupEntry(sideEntry->u.sideEntry.groupIdCounter,unitTable));
                                }
                                break;
            default : errorMsgUnknownElement(dec->line);
        }
        node = node->u.decList.tail;
    }
}


void checkVehicles(Absyn *vehicles,Table *table) {
    Absyn *node,*dec;

    node = vehicles->u.classTy.decList;
    while (!node->u.decList.isEmpty) {
        dec = node->u.decList.head;
        switch (dec->type) {
            case ABSYN_CLASSTY :
                               checkVehicleItem(dec,table);
                               break;

            case ABSYN_ARRAYTY :
            case ABSYN_STRTY :
            case ABSYN_NUMTY : break; //Just ignore these.
            default : errorMsgUnknownElement(dec->line);
        }
        node = node->u.decList.tail;
    }
}

void checkVehicleItem(Absyn *vehicleItem,Table *table) {
    Absyn *node,*dec;

    char *unitId = "";
    char *rankName = "PRIVATE";
    char *rankShortName = "";
    Sym *classname = NULL;
    boolean isLeader = FALSE;
    char *description = "";
    char *position = "";
    char *player = NULL;
    Roles roles;

    node = vehicleItem->u.classTy.decList;
    while (!node->u.decList.isEmpty) {
        dec = node->u.decList.head;
        switch (dec->type) {
            case ABSYN_CLASSTY :
            case ABSYN_ARRAYTY : break;  //Just ignore these.
            case ABSYN_STRTY :  if (symToStamp(dec->u.strTy.name) == symToStamp(itemRank)) {
                                    rankName = strListToString(dec->u.strTy.strList);
                                } else if (symToStamp(dec->u.strTy.name) == symToStamp(itemClass)) {
                                    classname = newSym(strListToString(dec->u.strTy.strList));
                                } else if (symToStamp(dec->u.strTy.name) == symToStamp(itemDescription)) {
                                    description = strListToString(dec->u.strTy.strList);
                                } else if (symToStamp(dec->u.strTy.name) == symToStamp(itemPlayer)) {
                                    player = strListToString(dec->u.strTy.strList);
                                }

                               break;
            case ABSYN_NUMTY : if (symToStamp(dec->u.numTy.name) == symToStamp(itemId)) {
                                    unitId = symToString(dec->u.numTy.value->u.num.value);
                                } else if (symToStamp(dec->u.numTy.name) == symToStamp(itemLeader)) {
                                    isLeader = (strcmp(symToString(dec->u.numTy.value->u.num.value),"1") == 0);
                                }
                                break;
            default : errorMsgUnknownElement(dec->line);
        }
        node = node->u.decList.tail;
    }

    if (player) {
        rankShortName = classnamesGetRankShort(rankName);
        rankName = classnamesGetRank(rankName);
        if (symToType(classname) == SYM_VEHICLE) {
            classnamesGetPlayerRoles(player,&roles);
            if (roles.Commander) {
              enter(table, vehicleItem->u.classTy.name, newUnitEntry(unitId,rankName,rankShortName,classname,isLeader,description,"Commander") );
            }
            if (roles.Driver) {
              enter(table, vehicleItem->u.classTy.name, newUnitEntry(unitId,rankName,rankShortName,classname,isLeader,description,"Driver") );
            }
            if (roles.Gunner) {
              enter(table, vehicleItem->u.classTy.name, newUnitEntry(unitId,rankName,rankShortName,classname,isLeader,description,"Gunner") );
            }

        } else {
            enter(table, vehicleItem->u.classTy.name, newUnitEntry(unitId,rankName,rankShortName,classname,isLeader,description,position) );
        }
    }
}

void errorMsgUnknownElement(int line) {
  error("unknown element in line %d",line);
}
