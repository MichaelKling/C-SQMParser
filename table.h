/*
 * table.h -- symbol table
 */


#ifndef _TABLE_H_
#define _TABLE_H_

#define ENTRY_KIND_SIDE		0
#define ENTRY_KIND_GROUP    1
#define ENTRY_KIND_UNIT		2

#include "types.h"
#include "sym.h"

typedef struct {
  int kind;
  union {
    struct {
      char *unitId;
      char *rankName;
      char *rankShortName;
      Sym *classname;
      boolean isLeader;
      char *description;
      char *position;
    } unitEntry;
    struct {
      int groupId;
      struct table *unitTable;
    } groupEntry;
    struct {
      int groupIdCounter;
      struct table *groupTable;
    } sideEntry;
  } u;
} Entry;

typedef struct bintree {
  Sym *sym;
  unsigned key;
  Entry *entry;
  struct bintree *left;
  struct bintree *right;
} Bintree;


typedef struct table {
  Bintree *bintree;
  struct table *upperLevel;
} Table;

Entry *newUnitEntry(char *unitId, char *rankName, char *rankShortName, Sym *classname, boolean isLeader, char *description,char *position);
Entry *newGroupEntry(int groupId, Table *unitTable);
Entry *newSideEntry(int groupIdCounter, Table *groupTable);

Table *newTable(Table *upperLevel);
Entry *enter(Table *table, Sym *sym, Entry *entry);
Entry *lookup(Table *table, Sym *sym);

void showEntry(Entry *entry, int n);
void showTable(Table *table, int n);


#endif /* _TABLE_H_ */
