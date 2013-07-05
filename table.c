/*
 * table.c -- symbol table
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "table.h"


Entry *newUnitEntry(int groupId, char *unitId, char *rankName, char *rankShortName, Sym *classname, boolean isLeader, char *description,char *position) {
    Entry *entry;
    entry = (Entry *) allocate(sizeof(Entry));
    entry->kind = ENTRY_KIND_UNIT;
    entry->u.unitEntry.groupId = groupId;
    entry->u.unitEntry.unitId = unitId;
    entry->u.unitEntry.rankName = rankName;
    entry->u.unitEntry.rankShortName = rankShortName;
    entry->u.unitEntry.classname = classname;
    entry->u.unitEntry.isLeader = isLeader;
    entry->u.unitEntry.description = description;
    entry->u.unitEntry.position = position;
    return entry;
}

Entry *newGroupEntry(int groupId, int groupSecondaryId, Table *unitTable) {
    Entry *entry;
    entry = (Entry *) allocate(sizeof(Entry));
    entry->kind = ENTRY_KIND_GROUP;
    entry->u.groupEntry.groupId = groupId;
    entry->u.groupEntry.groupSecondaryId = groupSecondaryId;
    entry->u.groupEntry.unitTable = unitTable;
    return entry;
}
Entry *newSideEntry(int groupIdCounter, Table *groupTable) {
    Entry *entry;
    entry = (Entry *) allocate(sizeof(Entry));
    entry->kind = ENTRY_KIND_SIDE;
    entry->u.sideEntry.groupIdCounter = groupIdCounter;
    entry->u.sideEntry.groupTable = groupTable;
    return entry;
}


Table *newTable(Table *upperLevel) {
  Table *table;

  table = (Table *) allocate(sizeof(Table));
  table->bintree = NULL;
  table->upperLevel = upperLevel;
  return table;
}


Entry *enter(Table *table, Sym *sym, Entry *entry) {
  unsigned key;
  Bintree *newtree;
  Bintree *current;
  Bintree *previous;

  key = symToStamp(sym);
  newtree = (Bintree *) allocate(sizeof(Bintree));
  newtree->sym = sym;
  newtree->key = key;
  newtree->entry = entry;
  newtree->left = NULL;
  newtree->right = NULL;
  if (table->bintree == NULL) {
    table->bintree = newtree;
  } else {
    current = table->bintree;
    while (1) {
      if (current->key == key) {
        /* symbol already in table */
        return NULL;
      }
      previous = current;
      if (current->key > key) {
        current = current->left;
      } else {
        current = current->right;
      }
      if (current == NULL) {
        if (previous->key > key) {
          previous->left = newtree;
        } else {
          previous->right = newtree;
        }
        break;
      }
    }
  }
  return entry;
}


static Entry *lookupBintree(Bintree *bintree, unsigned key) {
  while (bintree != NULL) {
    if (bintree->key == key) {
      return bintree->entry;
    }
    if (bintree->key > key) {
      bintree = bintree->left;
    } else {
      bintree = bintree->right;
    }
  }
  return NULL;
}


Entry *lookup(Table *table, Sym *sym) {
  unsigned key;
  Entry *entry;

  key = symToStamp(sym);
    entry = lookupBintree(table->bintree, key);
    if (entry != NULL) {
      return entry;
    }
  return NULL;
}

void showEntry(Entry *entry) {
  switch (entry->kind) {
    case ENTRY_KIND_SIDE:
      printf("side: \n");
      showSideEntry(entry);
      break;
    case ENTRY_KIND_GROUP:
      printf("group: \n");
      showGroupEntry(entry);
      break;
    case ENTRY_KIND_UNIT:
      printf("unit: \n");
      showUnitEntry(entry);
      break;
    default:
      error("unknown entry kind %d in showEntry", entry->kind);
  }
  printf("\n");
}


static void showBintree(Bintree *bintree) {
  if (bintree == NULL) {
    return;
  }
  showBintree(bintree->left);
  printf("  %-10s --> ", symToString(bintree->sym));
  showEntry(bintree->entry);
  showBintree(bintree->right);
}


void showTable(Table *table) {
  showBintree(table->bintree);
}
