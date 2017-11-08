SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o

#Choix compilateur
CC = gcc

#Options de compilation
CFLAGS = -g -O -I $(INC_DIR) -I ~/czmq/include/ -I ~/bin/libzmq/include
LDFLAGS =


EXEC=main

#Chemins
SRC_DIR=./src
OBJ_DIR=./build
INC_DIR=./include
BIN_DIR=./bin


all: $(EXEC)

$(EXEC): c s

#client
c: $(OBJ_DIR)/client.o
	$(CC) $< -o $(BIN_DIR)/client $(CFLAGS)

#serveur
s: $(OBJ_DIR)/server.o
	$(CC) $< -o $(BIN_DIR)/server $(CFLAGS)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)



.PHONY: all

clean:
	@rm -f $(OBJ_DIR)/*.o

mrproper: clean
	@rm -f $(BIN_DIR)/*
