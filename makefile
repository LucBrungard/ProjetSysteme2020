# PathToMake -f PathToMakeFile GCC=PathToGCC workspace=PathToProjectFolder
# -f, GCC and workspace are optionnal

GCC := gcc
workspace := 


all: reservTicket

reservTicket: $(workspace)obj/main.o
ifneq ("$(wildcard $(workspace)bin)", "")
	@echo "bin exists"
else
	mkdir $(workspace)bin
endif
	$(GCC) -o $(workspace)bin/reservTicket $(workspace)obj/main.o

$(workspace)obj/main.o: $(workspace)src/main.c
ifneq ("$(wildcard $(workspace)obj)", "")
	@echo "obj exists"
else
	mkdir $(workspace)obj
endif
	$(GCC) -o $(workspace)obj/main.o -c $(workspace)src/main.c

# exemple de fichier à compiler
#$(workspace)obj/dummy.o: $(workspace)src/dummy.c
#	$(GCC) -o $(workspace)obj/dummy.o -c $(workspace)src/dummy.c

#<id>: <dépendances>
#    <commandes shell>

#compiler: $(workspace)src/dummy.o   <- on demande "dummy.o"
#	des commandes shell...
