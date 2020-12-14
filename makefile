# PathToMake -f PathToMakeFile GCC=PathToGCC workspace=PathToProjectFolder
# -f, GCC and workspace are optionnal

GCC := gcc
workspace := 


all: client server


client: bin $(workspace)obj/client.o
	$(GCC) -o $(workspace)bin/client $(workspace)obj/client.o

server: bin $(workspace)obj/server.o
	$(GCC) -o $(workspace)bin/server $(workspace)obj/server.o -lpthread


$(workspace)obj/client.o: obj $(workspace)src/client.c
	$(GCC) -o $(workspace)obj/client.o -c $(workspace)src/client.c

$(workspace)obj/server.o: obj $(workspace)src/server.c
	$(GCC) -o $(workspace)obj/server.o -c $(workspace)src/server.c

bin:
ifeq ("$(wildcard $(workspace)bin)", "")
	mkdir $(workspace)bin
endif

obj:
ifeq ("$(wildcard $(workspace)obj)", "")
	mkdir $(workspace)obj
endif


# exemple de fichier à compiler
#$(workspace)obj/dummy.o: $(workspace)src/dummy.c
#	$(GCC) -o $(workspace)obj/dummy.o -c $(workspace)src/dummy.c

#<id>: <dépendances>
#    <commandes shell>

#compiler: $(workspace)src/dummy.o   <- on demande "dummy.o"
#	des commandes shell...
