# PathToMake -f PathToMakeFile GCC=PathToGCC workspace=PathToProjectFolder
# -f, GCC and workspace are optionnal

GCC := gcc
workspace := 


all: client server


client: bin

server: bin $(workspace)obj/server/Server.o $(workspace)obj/server/Client.o $(workspace)obj/server/main.o
	$(GCC) -o $(workspace)bin/server $(workspace)obj/server/Server.o $(workspace)obj/server/Client.o $(workspace)obj/server/main.o -lpthread

$(workspace)obj/server/Server.o: obj $(workspace)src/server/Server.c
	$(GCC) -o $(workspace)obj/server/Server.o -c $(workspace)src/server/Server.c

$(workspace)obj/server/Client.o: obj $(workspace)src/server/Client.c
	$(GCC) -o $(workspace)obj/server/Client.o -c $(workspace)src/server/Client.c

$(workspace)obj/server/main.o: obj $(workspace)src/server/main.c
	$(GCC) -o $(workspace)obj/server/main.o -c $(workspace)src/server/main.c

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


# exemple de fichier à compiler
#$(workspace)obj/dummy.o: $(workspace)src/dummy.c
#	$(GCC) -o $(workspace)obj/dummy.o -c $(workspace)src/dummy.c

#<id>: <dépendances>
#    <commandes shell>

#compiler: $(workspace)src/dummy.o   <- on demande "dummy.o"
#	des commandes shell...
