CC= gcc
RM= rm -vf
CFLAGS= -lm -g
SRCDIR=.
EXECDIR= .
SRCFILES= server.c client.c
EXECPATH= $(patsubst %.c, $(EXECDIR)/%, $(SRCFILES))

all: $(EXECPATH)

$(EXECDIR)/server: $(SRCDIR)/server.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/server $(SRCDIR)/server.c

$(EXECDIR)/client: $(SRCDIR)/client.c
	$(CC) $(CFLAGS) -o $(EXECDIR)/client $(SRCDIR)/client.c

.PHONY: clean
clean:
	$(RM) $(EXECDIR)/server $(EXECDIR)/client $(EXECPATH)/~*
