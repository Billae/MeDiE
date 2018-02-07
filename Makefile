include Makefile.inc

SHELL = /bin/sh

#Chemins
SRC_DIR=./src
OBJ_DIR=./build
INC_DIR=./include
BIN_DIR=./bin


EXEC=main

all: $(EXEC)

$(EXEC): c s

#client
c: $(OBJ_DIR)/client.o $(OBJ_DIR)/request.o $(OBJ_DIR)/distribution.o 
	@$(CC) $^ -o $(BIN_DIR)/client $(CFLAGS) $(LDFLAGS)

#serveur
s: $(OBJ_DIR)/server.o $(OBJ_DIR)/generic_storage.o $(INC_DIR)/protocol.h
	@$(CC) $^ -o $(BIN_DIR)/server $(CFLAGS) $(LDFLAGS)



$(OBJ_DIR)/request.o: $(SRC_DIR)/request.c $(INC_DIR)/request.h $(INC_DIR)/distribution.h $(INC_DIR)/protocol.h
	@$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	@$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: all

clean:
	@rm -f $(OBJ_DIR)/*.o

mrproper: clean
	@rm -f $(BIN_DIR)/*
