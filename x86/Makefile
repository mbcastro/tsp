TSPDIR=$(HOME)/projects/tsp/x86

OBJ=$(TSPDIR)/posix/obj
SRC=$(TSPDIR)
BIN=$(TSPDIR)/posix/bin

ARCH= -DMT -O6
LIBS=-lpthread
CC2=cc -c $(ARCH) -o $@ $(SRC)/$(@F:.o=.c) -pthread 

all: $(BIN)/tsp 

LINK=cc -pthread -o $@ $(OBJ)/$(@F).o 

# executables :

$(BIN)/tsp: $(OBJ)/tsp.o $(OBJ)/job.o $(OBJ)/main.o $(LIB)
	$(LINK) $(OBJ)/job.o  $(OBJ)/main.o -lm

# objects :


$(OBJ)/tsp.o: $(SRC)/tsp.c $(SRC)/types.h $(SRC)/tsp.h
	$(CC2) 

$(OBJ)/main.o: $(SRC)/main.c $(SRC)/types.h $(SRC)/tsp.h 
	$(CC2) 

$(OBJ)/job.o: $(SRC)/job.c $(SRC)/types.h $(SRC)/tsp.h 
	$(CC2)  




# clean

clean:
	rm -rf $(OBJ)/*.o $(BIN)/*
