.SUFFIXES: .o .c .h
.PHONY: all clean install uninstall

CFLAGS=-O2 -Wall -fno-builtin --pedantic
LDFLAGS=
CC=cc

INSTALL=install

LOCALD=$(HOME)/.qed

BINDIR=$(LOCALD)/bin
MANDIR=$(LOCALD)/man/man1
LIBDIR=$(LOCALD)/lib

QLIB=\
  q/*

HDRS=\
  vars.h \
  unix.h \
  funcs.h \
  utf.h \
  qed.h \

UNITS=\
  alloc \
  address \
  blkio \
  com \
  getchar \
  getfile \
  glob \
  main \
  misc \
  move \
  pattern \
  putchar \
  setaddr \
  string \
  subs \
  u \
  utf \

OBJS=$(UNITS:=.o)
SRCS=$(UNITS:=.c)

.c.o:
	$(CC) -c $(CFLAGS) $<

all: a.out

a.out: $(HDRS) $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS)

clean: 
	rm -f a.out *.o

install: a.out qed.1 $(QLIB) env
	$(INSTALL) -d $(BINDIR) $(LIBDIR) $(MANDIR)
	$(INSTALL) -s a.out $(BINDIR)/qed
	$(INSTALL) qed.1 $(MANDIR)
	$(INSTALL) $(QLIB) $(LIBDIR)
	$(INSTALL) env $(LOCALD)
	@echo ""
	@echo "Add the following line to your shell profile:"
	@echo ""
	@echo ". \"$(LOCALD)/env\""
	@echo ""
	@echo "Source the file: \"$(LOCALD)/env\" to update your current environemnt"

uninstall:
	rm -rf $(LOCALD)
	@echo ""
	@echo "Remove the following line from your shell profile:"
	@echo ""
	@echo ". \"$(LOCALD)/env\""
	@echo ""
