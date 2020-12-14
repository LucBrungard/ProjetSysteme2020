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
