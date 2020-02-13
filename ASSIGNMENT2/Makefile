CC= gcc
RM= rm -vf
CFLAGS= -lm -g -pthread
SRCDIR=.
EXECDIR= .
SRCFILES= serverGBN.c clientGBN.c serverSR.c clientSR.c
EXECPATH= $(patsubst %.c, $(EXECDIR)/%, $(SRCFILES))

all: $(EXECPATH)

$(EXECDIR)/serverGBN: $(SRCDIR)/serverGBN.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/serverGBN $(SRCDIR)/serverGBN.c

$(EXECDIR)/clientGBN: $(SRCDIR)/clientGBN.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/clientGBN $(SRCDIR)/clientGBN.c

$(EXECDIR)/serverSR: $(SRCDIR)/serverSR.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/serverSR $(SRCDIR)/serverSR.c

$(EXECDIR)/clientSR: $(SRCDIR)/clientSR.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/clientSR $(SRCDIR)/clientSR.c

.PHONY: clean
clean:
	$(RM) $(EXECDIR)/serverGBN $(EXECDIR)/clientGBN $(EXECDIR)/serverSR $(EXECDIR)/serverSR $(EXECPATH)/~*
