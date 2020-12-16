# PathToMake -f PathToMakeFile GCC=PathToGCC workspace=PathToProjectFolder
# -f, GCC and workspace are optionnal

CC := gcc
workspace := 
FLAGS := -g


all: client server


client: bin $(workspace)obj/client/Client.o $(workspace)obj/client/main.o
	$(CC) $(FLAGS) -o $(workspace)bin/client $(workspace)obj/client/main.o $(workspace)obj/client/Client.o

server: bin $(workspace)obj/server/Server.o $(workspace)obj/server/Client.o $(workspace)obj/server/main.o
	$(CC) $(FLAGS) -o $(workspace)bin/server $(workspace)obj/server/Server.o $(workspace)obj/server/Client.o $(workspace)obj/server/main.o -lpthread

$(workspace)obj/server/Server.o: obj $(workspace)src/server/Server.c
	$(CC) $(FLAGS) -o $(workspace)obj/server/Server.o -c $(workspace)src/server/Server.c

$(workspace)obj/server/Client.o: obj $(workspace)src/server/Client.c
	$(CC) $(FLAGS) -o $(workspace)obj/server/Client.o -c $(workspace)src/server/Client.c

$(workspace)obj/server/main.o: obj $(workspace)src/server/main.c
	$(CC) $(FLAGS) -o $(workspace)obj/server/main.o -c $(workspace)src/server/main.c

$(workspace)obj/client/main.o: obj $(workspace)src/client/main.c
	$(CC) $(FLAGS) -o $(workspace)obj/client/main.o -c $(workspace)src/client/main.c

$(workspace)obj/client/Client.o: obj $(workspace)src/client/Client.c
	$(CC) $(FLAGS) -o $(workspace)obj/client/Client.o -c $(workspace)src/client/Client.c

bin:
ifeq ("$(wildcard $(workspace)bin)", "")
	mkdir $(workspace)bin
endif

obj:
ifeq ("$(wildcard $(workspace)obj)", "")
	mkdir $(workspace)obj
endif
ifeq ("$(wildcard $(workspace)obj/server)", "")
	mkdir $(workspace)obj/server
endif
ifeq ("$(wildcard $(workspace)obj/client)", "")
	mkdir $(workspace)obj/client
endif
