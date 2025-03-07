CC = gcc
CFLAGS = -Wall -g
SRCDIR = src
OBJDIR = obj

all: $(OBJDIR) blueis blueis_server client_example

blueis: $(OBJDIR)/main.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis.o
	gcc -o blueis $(OBJDIR)/main.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis.o

blueis_server: $(OBJDIR)/blueis_server.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis.o
	gcc -o blueis_server $(OBJDIR)/blueis_server.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis.o

client_example: $(OBJDIR)/client_example.o $(OBJDIR)/blueis_client.o $(OBJDIR)/blueis.o $(OBJDIR)/blueis_storage.o
	gcc -o client_example $(OBJDIR)/client_example.o $(OBJDIR)/blueis_storage.o $(OBJDIR)/blueis_client.o $(OBJDIR)/blueis.o

$(OBJDIR)/blueis_storage.o: $(SRCDIR)/blueis_storage.c $(SRCDIR)/blueis_storage.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis_storage.c -o $(OBJDIR)/blueis_storage.o

$(OBJDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(OBJDIR)/main.o

$(OBJDIR)/blueis_server.o: $(SRCDIR)/blueis_server.c 
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis_server.c -o $(OBJDIR)/blueis_server.o

$(OBJDIR)/blueis_client.o: $(SRCDIR)/blueis_client.c 
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis_client.c -o $(OBJDIR)/blueis_client.o

$(OBJDIR)/client_example.o: $(SRCDIR)/client_example.c 
	$(CC) $(CFLAGS) -c $(SRCDIR)/client_example.c -o $(OBJDIR)/client_example.o

$(OBJDIR)/blueis.o: $(SRCDIR)/blueis.c $(SRCDIR)/blueis.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/blueis.c -o $(OBJDIR)/blueis.o

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	rm -f blueis & rm -rf $(OBJDIR) & rm -f blueis_server & rm -f blueis & rm -f client_example
