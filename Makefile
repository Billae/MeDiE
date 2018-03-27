include Makefile.inc

SHELL = /bin/sh

#Chemins
SRC_DIR=./src
OBJ_DIR=./build
INC_DIR=./include
BIN_DIR=./bin


EXEC=main

all: $(EXEC)

$(EXEC): c s0 s1 s2 s3

#client
c: $(OBJ_DIR)/client.o $(OBJ_DIR)/request.o $(OBJ_DIR)/distribution.o $(OBJ_DIR)/murmur3.o 
	@$(CC) $^ -o $(BIN_DIR)/client $(CFLAGS) $(LDFLAGS)

#serveurs
s0: $(OBJ_DIR)/server0.o $(OBJ_DIR)/generic_storage.o $(INC_DIR)/protocol.h
	@$(CC) $^ -o $(BIN_DIR)/server0 $(CFLAGS) $(LDFLAGS)

s1: $(OBJ_DIR)/server1.o $(OBJ_DIR)/generic_storage.o $(INC_DIR)/protocol.h
	@$(CC) $^ -o $(BIN_DIR)/server1 $(CFLAGS) $(LDFLAGS)

s2: $(OBJ_DIR)/server2.o $(OBJ_DIR)/generic_storage.o $(INC_DIR)/protocol.h
	@$(CC) $^ -o $(BIN_DIR)/server2 $(CFLAGS) $(LDFLAGS)

s3: $(OBJ_DIR)/server3.o $(OBJ_DIR)/generic_storage.o $(INC_DIR)/protocol.h
	@$(CC) $^ -o $(BIN_DIR)/server3 $(CFLAGS) $(LDFLAGS)




$(OBJ_DIR)/request.o: $(SRC_DIR)/request.c $(INC_DIR)/request.h $(INC_DIR)/distribution.h $(INC_DIR)/protocol.h
	@$(CC) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	@$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: all

clean:
	@rm -f $(OBJ_DIR)/*.o
	@rm -f dataStore/*

mrproper: clean
	@rm -f $(BIN_DIR)/*
	@rm -f dataStore/*
