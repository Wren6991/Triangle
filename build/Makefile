SRCDIR=../src
LIBS=SDL2
APPNAME=triangle

CC=gcc
LD=gcc
CCFLAGS=-Og -Wall -Wextra
LDFLAGS=

###

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(addsuffix .o,$(notdir $(SRCS)))

.SUFFIXES:
.PHONY: all clean run

all: $(APPNAME)

clean:
	rm -f $(OBJS) $(APPNAME)

run: all
	./$(APPNAME)

%.c.o: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) -c $(<) -o $(@)

$(APPNAME): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(addprefix -l,$(LIBS)) -o $(APPNAME)
