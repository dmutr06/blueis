CC = gcc
CFLAGS = -Wall -g
SRCDIR = src
OBJDIR = obj

all: $(OBJDIR) blueis 

blueis: $(OBJDIR)/main.o $(OBJDIR)/blueis_storage.o
	gcc -o blueis $(OBJDIR)/main.o $(OBJDIR)/blueis_storage.o

$(OBJDIR)/blueis_storage.o: $(SRCDIR)/blueis_storage.c $(SRCDIR)/blueis_storage.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis_storage.c -o $(OBJDIR)/blueis_storage.o

$(OBJDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(OBJDIR)/main.o

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	rm -f blueis & rm -rf $(OBJDIR)
