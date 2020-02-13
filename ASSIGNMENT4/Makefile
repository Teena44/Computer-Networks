CC= gcc
RM= rm -vf
CFLAGS= -lm -g -pthread
SRCDIR=.
EXECDIR= .
SRCFILES= TCP.c
EXECPATH= $(patsubst %.c, $(EXECDIR)/%, $(SRCFILES))

all: $(EXECPATH)

$(EXECDIR)/TCP: $(SRCDIR)/TCP.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/TCP $(SRCDIR)/TCP.c -lm

.PHONY: clean
clean:
	$(RM) $(EXECDIR)/TCP $(EXECPATH)/~*
