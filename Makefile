TSPDIR=$(HOME)/projects/tsp/x86

OBJ=$(TSPDIR)/obj
SRC=$(TSPDIR)
BIN=$(TSPDIR)/bin

ARCH= -DMT -O3 -DDEBUG -g
LIBS=-lpthread
CC2=cc -c $(ARCH) -o $@ $(SRC)/$(@F:.o=.c) -Wall -pthread 

all: $(BIN)/tsp 

LINK=cc -pthread -o $@ $(OBJ)/$(@F).o 

# executables :

$(BIN)/tsp: $(OBJ)/tsp.o $(OBJ)/job.o $(OBJ)/main.o $(OBJ)/timer.o $(OBJ)/defs.o
	$(LINK) $(OBJ)/job.o  $(OBJ)/main.o $(OBJ)/timer.o $(OBJ)/defs.o -lm

# objects :


$(OBJ)/tsp.o: $(SRC)/tsp.c 
	$(CC2) 

$(OBJ)/main.o: $(SRC)/main.c 
	$(CC2) 

$(OBJ)/job.o: $(SRC)/job.c 
	$(CC2)  

$(OBJ)/timer.o: $(SRC)/timer.c 
	$(CC2)  

$(OBJ)/defs.o: $(SRC)/defs.c 
	$(CC2)  	


# clean

clean:
	rm -rf $(OBJ)/*.o $(BIN)/*
