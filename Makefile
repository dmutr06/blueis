CC = gcc
CFLAGS = -Wall -g
SRCDIR = src
OBJDIR = obj

all: $(OBJDIR) blueis 

blueis: $(OBJDIR)/main.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis.o
	gcc -o blueis $(OBJDIR)/main.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis.o

$(OBJDIR)/blueis_storage.o: $(SRCDIR)/blueis_storage.c $(SRCDIR)/blueis_storage.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis_storage.c -o $(OBJDIR)/blueis_storage.o

$(OBJDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(OBJDIR)/main.o

$(OBJDIR)/blueis.o: $(SRCDIR)/blueis.c $(SRCDIR)/blueis.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis.c -o $(OBJDIR)/blueis.o

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	rm -f blueis & rm -rf $(OBJDIR)
