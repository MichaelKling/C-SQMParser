#
# Makefile for SPL compiler
#

CC = gcc
BISON = bison
FLEX = flex
CFLAGS = -Wall -Wno-unused -g
LDFLAGS = -g
LDLIBS = -lm
# semant.c varalloc.c codegen.c
SRCS = main.c utils.c parser.tab.c lex.yy.c absyn.c sym.c types.c table.c classnames/classnames.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = sqmParser

.PHONY:		all depend clean dist-clean

all: depend $(BIN)

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

parser.tab.c:	parser.y
		$(BISON) -v -d -t parser.y

lex.yy.c:	scanner.l
		$(FLEX) scanner.l


-include depend.mak

depend:	parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o
		rm lex.yy.c
		rm parser.tab.c

dist-clean:	clean
		rm -f $(BIN) parser.tab.c parser.tab.h lex.yy.c depend.mak

cleanall: dist-clean
