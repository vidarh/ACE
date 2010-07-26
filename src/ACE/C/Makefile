
OBJS=alloc.o basfun.o declare.o expr.o file.o gfx.o lex.o libfunc.o menu.o misc.o parse.o print.o serial.o sub.o symvar.o window.o \
	assign.o control.o event.o factor.o gadget.o iff.o memory.o message.o opt.o parsevar.o screen.o statement.o sym.o ver.o ansi_compat.o codegen.o

CFLAGS=-I. -DANSI_COMPAT -g -ansi -pedantic -Wall  -W -Wcast-qual -std=c99 #-Wconversion -W shadow -Wwrite-strings
LDFLAGS=-lm

all: ace

ace: $(OBJS)
	$(CC) -o ace $(LDFLAGS) $(OBJS)

$(OBJS): acedef.h

lex.c: lexvar.c

clean:
	rm -f *.o *~ \#*
