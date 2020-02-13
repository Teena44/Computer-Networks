CC= gcc
RM= rm -vf
CFLAGS= -lm -g -pthread
SRCDIR=.
EXECDIR= .
SRCFILES= Assignment3.c
EXECPATH= $(patsubst %.c, $(EXECDIR)/%, $(SRCFILES))

all: $(EXECPATH)

$(EXECDIR)/Assignment3: $(SRCDIR)/Assignment3.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/Assignment3 $(SRCDIR)/Assignment3.c

.PHONY: clean
clean:
	$(RM) $(EXECDIR)/Assignment3 $(EXECPATH)/~*
