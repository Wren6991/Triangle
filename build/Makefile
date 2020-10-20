SRCDIR=../src
LIBS=SDL2 m
APPNAME=triangle

CC=gcc
LD=gcc
CCFLAGS=-O2 -Wall -Wextra
LDFLAGS=

###

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(addsuffix .o,$(notdir $(SRCS)))

.SUFFIXES:
.PHONY: all clean run

all: $(APPNAME)

clean:
	rm -f $(OBJS) $(APPNAME) teapot.tri

run: all
	../scripts/obj2tri ../assets/teapot.obj teapot.tri
	./$(APPNAME) teapot.tri

%.c.o: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) -c $(<) -o $(@)

$(APPNAME): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(addprefix -l,$(LIBS)) -o $(APPNAME)
