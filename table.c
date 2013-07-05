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


Entry *newUnitEntry(char *unitId, char *rank, char *rankName, char *rankShortName, Sym *classname, boolean isLeader, char *description,char *position) {
    Entry *entry;
    entry = (Entry *) allocate(sizeof(Entry));
    entry->kind = ENTRY_KIND_UNIT;
    entry->u.unitEntry.unitId = unitId;
    entry->u.unitEntry.rank = rank;
    entry->u.unitEntry.rankName = rankName;
    entry->u.unitEntry.rankShortName = rankShortName;
    entry->u.unitEntry.classname = classname;
    entry->u.unitEntry.isLeader = isLeader;
    entry->u.unitEntry.description = description;
    entry->u.unitEntry.position = position;
    return entry;
}

Entry *newGroupEntry(int groupId, Table *unitTable) {
    Entry *entry;
    entry = (Entry *) allocate(sizeof(Entry));
    entry->kind = ENTRY_KIND_GROUP;
    entry->u.groupEntry.groupId = groupId;
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
  table->size = 0;
  table->upperLevel = upperLevel;
  return table;
}


Entry *enter(Table *table, Sym *sym, Entry *entry) {
  unsigned key;
  Bintree *newtree;
  Bintree *current;
  Bintree *previous;

  key = symToCounter(sym);
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
  table->size++;
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

  key = symToCounter(sym);
    entry = lookupBintree(table->bintree, key);
    if (entry != NULL) {
      return entry;
    }
  return NULL;
}

static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}

void showEntry(Entry *entry, int n) {
  switch (entry->kind) {
    case ENTRY_KIND_SIDE:
      indent(n);
      printf("Side: \n");
      showSideEntry(entry,++n);
      break;
    case ENTRY_KIND_GROUP:
      indent(n);
      printf("Group: \n");
      showGroupEntry(entry,++n);
      break;
    case ENTRY_KIND_UNIT:
      indent(n);
      printf("Unit: \n");
      showUnitEntry(entry,++n);
      break;
    default:
      error("unknown entry kind %d in showEntry", entry->kind);
  }
  printf("\n");
}


static void showBintree(Bintree *bintree, int n) {
  if (bintree == NULL) {
    return;
  }
  showBintree(bintree->left,n);
  indent(n);
  printf("TableEntry: %s --> %s\n", symToString(bintree->sym),symToValue(bintree->sym));
  showEntry(bintree->entry,n+1);
  showBintree(bintree->right,n);
}


void showTable(Table *table, int n) {
  indent(n);printf("Table: (%d items)\n",table->size);
  showBintree(table->bintree,++n);
}
